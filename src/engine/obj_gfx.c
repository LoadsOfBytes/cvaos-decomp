/*
 * Object graphics: sprite slots, part drawing (normal / affine / single), tile loading and the save / restore of the object graphics state.
 */
#include "structs/render_slots.h"
#include "core.h"
#include "bg_and_collision.h"
#include "gameplay.h"
#include "arm_callbacks.h"
#include "gba.h"
#include "syscalls.h"
#include "constants/main.h"
#include "data/data_0E0334.h"
#include "data/data_0E3464.h"
#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "graphics_inline.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "data/data_0E1CD0.h"
#include "data/data_118B48.h"

/* Upstream functions 0x08039340..0x0803981F from the previously unlinked
 * this file, now linked as matching C. */

/**
 * @brief 39340 | Reset the graphics cache
 *
 * All 16 entries go to the free list (tileSlot -1); the frame-set list
 * objGfxSetFrames points at the six objGfxFrameBuffers buffers.
 */
void ObjGfx_Init(void)
{
    s32 i;

    DMA_FILL_32(3, 0, &gEwramData->objGfx, 0x140);
    DMA_FILL_32(3, 0, &gEwramData->objGfxSaved, 0x16C);

    for (i = 0; i < 0x10; i++)
    {
        gEwramData->objGfx[i].prev = i - 1;
        gEwramData->objGfx[i].next = i + 1;
        gEwramData->objGfx[i].tileSlot = -1;
    }

    gEwramData->objGfx[0].prev = -1;
    gEwramData->objGfx[0xF].prev = -1;

    gEwramData->objGfxSet = 0;
    gEwramData->objGfxSetList = 2;

    for (i = 0; i < 6; i++)
    {
        gEwramData->objGfxSetFrames[i] = &gEwramData->objGfxFrameBuffers[i]; // TODO: storing address?
    }
}

/**
 * @brief 3943C | Move the entry that owns a tile slot back to the free list
 *
 * Only the cache entry is cleared; the tile slots are freed by the caller
 * (see ObjGfx_Unload).
 *
 * @param tileSlot First tile slot of the entry (0-15)
 */
void ObjGfx_Release(s16 tileSlot)
{
    s32 entry;
    s16 freeHead;
    s16 prev;
    s16 next;
    s32 savedPrev;
    s32 savedNext;

    if (tileSlot > 0xFU)
    {
        return;
    }

    entry = gEwramData->objGfxUsedHead;
    if (entry != -1)
    {
        while (gEwramData->objGfx[entry].tileSlot != tileSlot)
        {
            do {} while(0); /* FAKEMATCH: empty do/while(0) only adds a loop note for the register allocator */
            entry = gEwramData->objGfx[entry].next;
            if (entry == -1)
            {
                return;
            }
        }
    }

    freeHead = gEwramData->objGfxFreeHead;
    prev = gEwramData->objGfx[entry].prev;
    next = gEwramData->objGfx[entry].next;
    if (entry == gEwramData->objGfxUsedHead)
    {
        gEwramData->objGfxUsedHead = next;
        if (next != -1)
        {
            gEwramData->objGfx[next].prev = -1;
        }
    }
    else
    {
        gEwramData->objGfx[prev].next = next;
        if (next != -1)
        {
            gEwramData->objGfx[next].prev = prev;
        }
    }
    if (freeHead != -1)
    {
        gEwramData->objGfx[freeHead].prev = entry;
    }
    gEwramData->objGfx[entry].next = freeHead;
    gEwramData->objGfx[entry].prev = -1;
    gEwramData->objGfxFreeHead = entry;
    if (gEwramData->objGfx[entry].kind == 2)
    {
        gEwramData->objGfxSet = 0;
    }

    savedPrev = gEwramData->objGfx[entry].prev;
    savedNext = gEwramData->objGfx[entry].next;
    DMA_FILL_16(3, 0, &gEwramData->objGfx[entry], 0x14);
    gEwramData->objGfx[entry].prev = savedPrev;
    gEwramData->objGfx[entry].next = savedNext;
    gEwramData->objGfx[entry].tileSlot = -1;
}

/**
 * @brief 39614 | Take a sprite slot from the free list
 *
 * @param sheet Sprite sheet (animation data)
 * @param tileSlot First tile slot of the loaded graphics
 * @return u8 Sprite slot, 0xFF if none is free
 */
u8 Sprite_AllocSlot(void *sheet, s32 tileSlot)
{
    s32 gfxEntry;
    s16 usedHead;
    s16 slot;

    gfxEntry = ObjGfx_FindEntry(tileSlot);

    if (gEwramData->spriteFreeHead != -1)
    {
        slot = gEwramData->spriteFreeHead;
        usedHead = gEwramData->spriteUsedHead;
        gEwramData->spriteFreeHead = gEwramData->spriteSlots[slot].next;
        if (gEwramData->spriteSlots[slot].next != -1)
        {
            gEwramData->spriteSlots[gEwramData->spriteSlots[slot].next].prev = -1;
        }
        if (usedHead != -1)
        {
            gEwramData->spriteSlots[usedHead].prev = slot;
        }
        gEwramData->spriteUsedHead = slot;
        gEwramData->spriteSlots[slot].next = usedHead;
        gEwramData->spriteSlots[slot].prev = -1;
        gEwramData->spriteSlots[slot].gfxEntry = gfxEntry;
        gEwramData->spriteSlots[slot].sheet = sheet;
        gEwramData->spriteSlots[slot].tileSlot = tileSlot;
        return slot;
    }
    return -1;
}

/**
 * @brief 39748 | Unpack the frames of a type 3 frame set into objGfxFrameBuffers
 *
 * Frames stored raw are copied, LZ77 frames are decompressed; the set's
 * header bytes 1-3 are copied to objGfxSetListHeader.
 *
 * @param set Frame set (unk_1 = frame count, unk_4 = frame pointers)
 */
void ObjGfx_DecompressSet(struct EwramData_unk19404 *set)
{
    s32 i;
    struct ObjGfxFrameResource *src;
    struct EwramData_unk19404 *dst;
    s32 length;

    gEwramData->objGfxSet = set;

    for (i = 0; i < set->unk_1; i++)
    {
        src = set->unk_4[i];
        dst = &gEwramData->objGfxFrameBuffers[i];
        // temp_r1 = src->unk_0;
        length = src->widthTiles * src->heightTiles * src->bitsPerPixel * 8 + 4;
        switch (src->kind)
        {
            case 0:
                DMA_COPY_16(3, src, dst, length);
                break;

            case 1:
                LZ77UnCompWram((u32 *) src->compressedData, dst);
                dst->unk_0 = 0;
                dst->unk_1 = src->bitsPerPixel;
                dst->unk_2 = src->widthTiles;
                dst->unk_3 = src->heightTiles;
                break;
        }
    }

    gEwramData->objGfxSetListHeader[0] = set->unk_1;
    gEwramData->objGfxSetListHeader[1] = set->unk_2;
    gEwramData->objGfxSetListHeader[2] = set->unk_3;
}

/* Draw one entity's sprite parts through an affine matrix (rotated/scaled OAM). */
struct Matrix2_2 { s32 m[4]; };
struct ObjEntry;
extern void (*gObjAffineGetMatrixFunc)(struct Matrix2_2 *, s32);
extern s32 (*gObjAffineGetFlipXFunc)(s32);
extern s32 (*gObjAffineGetFlipYFunc)(s32);
extern u16 *(*gObjAllocManyFunc)(u32);
extern void (*gObjAddToLayerFunc)(u16 *, u32);
extern void (*gMatrixMultiplyVec2Func)(s32 *, struct Matrix2_2 *, s32 *);
extern s32 (*gObjTilesGetTileFunc)(s32, s32, s32);

#define WHOLE(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

static inline void Mirror(s32 *dst, s32 *src, s32 *origin, s32 shape, s32 size, s32 flipX, s32 flipY)
{
        if (flipX)
        {
            s32 extent = sUnk_080E29A0[shape][size][0];
            dst[0] = origin[0] * 2 - src[0] - extent * 2;
        }
        else
            dst[0] = src[0];
        if (flipY)
        {
            s32 extent = sUnk_080E29A0[shape][size][1];
            dst[1] = origin[1] * 2 - src[1] - extent * 2;
        }
        else
            dst[1] = src[1];
}

/**
 * @brief 39820 | Draw a multi-part sprite (Thumb version)
 *
 * Builds one OAM object per part of the current frame (entity->animFrame) with
 * the entity's flips, palette, OBJ mode and mosaic, and links them into layer
 * entity->spriteLayer. The ARM copy Sprite_DrawPartsArm is used at runtime.
 *
 * @param entity Entity with a sprite
 * @return s32 1 if drawn, 0 without a sheet or when out of objects
 */
s32 Sprite_DrawParts(struct EwramData_EntityData *entity)
{
    s32 origin[2];
    s32 position[2];
    s32 graphics;
    s32 i;
    u32 baseAttributes;
    s32 paletteOffset = 0;
    u16 *slot;
    struct SpriteSheet *animation;
    struct SpriteFrame *frame;
    struct SpritePart *part;
    u16 *output;
    s32 shape, size;
    s32 flipX, flipY;
    s32 id;
    struct EwramData *ew;
    s32 count;

    ew = gEwramData;
    id = entity->spriteSlot;
    graphics = ew->spriteSlots[id].tileSlot;
    animation = ew->spriteSlots[(u8)id].sheet;
    if (animation == NULL)
        return 0;
    switch (entity->animMode)
    {
    case 0:
    case 1:
        break;
    case 2:
        paletteOffset = gEwramData->objTileSlots[(s16)graphics].size;
        break;
    }
    if (entity->animFrame < animation->frameCount)
        frame = &animation->frames[entity->animFrame];
    else
        frame = animation->frames;
    count = frame->partCount;
    part = frame->parts;
    origin[0] = frame->originX << 16;
    origin[1] = frame->originY << 16;
    baseAttributes = entity->mosaic << 12;
    baseAttributes |= entity->objMode << 10;
    slot = gObjAllocManyFunc(count);
    if (slot == NULL)
        return 0;
    for (i = 0; i < count; i++, part++, slot += 6)
    {
        {
            output = slot;
            shape = part->shape;
            size = part->size;
            flipX = entity->flipX;
            flipY = entity->flipY;
            position[0] = part->x << 16;
            position[1] = part->y << 16;
            if (flipX | flipY)
                Mirror(position, position, origin, shape, size, flipX, flipY);
            *output++ = (shape << 14) | baseAttributes | (WHOLE(position[1] + entity->posY.whole) & 0xFF);
            *output++ = (size << 14) | ((flipY ^ (part->flip & 1)) << 13) | (((part->flip & 2) ? flipX ^ 1 : flipX) << 12) | (WHOLE(position[0] + entity->posX.whole) & 0x1FF);
            *output = ((entity->palette + (part->paletteOffset & entity->paletteMask)) << 12)
                | gObjTilesGetTileFunc(part->gfxFrame ? graphics + paletteOffset : graphics, part->tileX, part->tileY);
            gObjAddToLayerFunc(slot, entity->spriteLayer);
        }
    }
    return 1;
}

/* Draw one entity's sprite parts through an affine matrix (rotated/scaled OAM). */
struct ObjEntry;

static inline void Rotate(s32 *dst, s32 *src, s32 *origin, struct Matrix2_2 *matrix, s32 shape, s32 size, s32 doubled)
{
    s32 temp[2];
    s32 extentX = sUnk_080E29A0[shape][size][0];
    s32 extentY = sUnk_080E29A0[shape][size][1];

    if (doubled)
    {
        temp[0] = src[0] - extentX;
        temp[1] = src[1] - extentY;
        extentX <<= 1;
        extentY <<= 1;
    }
    else
    {
        temp[0] = src[0];
        temp[1] = src[1];
    }
    temp[0] = extentX - origin[0] + temp[0];
    temp[1] = extentY - origin[1] + temp[1];
    gMatrixMultiplyVec2Func(temp, matrix, temp);
    dst[0] = temp[0] - extentX;
    dst[1] = temp[1] - extentY;
}

/**
 * @brief 39A6C | Draw a multi-part sprite through an affine set (Thumb version)
 *
 * Builds one OAM object per part of the current frame (entity->animFrame) with
 * the entity's flips, palette, OBJ mode and mosaic, and links them into layer
 * entity->spriteLayer. Part positions are rotated/scaled by the matrix of entity->affineIndex.
 * The ARM copy Sprite_DrawPartsAffineArm is used at runtime.
 *
 * @param entity Entity with a sprite
 * @return s32 1 if drawn, 0 without a sheet or when out of objects
 */
s32 Sprite_DrawPartsAffine(struct EwramData_EntityData *entity)
{
    struct Matrix2_2 matrix;
    s32 origin[2];
    s32 position[2];
    s32 result[2];
    s32 graphics;
    s32 i;
    u32 baseAttributes;
    s32 flipX, flipY;
    s32 paletteOffset = 0;
    s32 affine;
    u16 *slot;
    s32 shape, size;
    struct SpriteSheet *animation;
    struct SpriteFrame *frame;
    struct SpritePart *part;
    u16 *output;
    s32 id;
    struct EwramData *ew;
    s32 count;

    ew = gEwramData;
    id = entity->spriteSlot;
    graphics = ew->spriteSlots[id].tileSlot;
    animation = ew->spriteSlots[(u8)id].sheet;
    if (animation == NULL)
        return 0;
    switch (entity->animMode)
    {
    case 0:
    case 1:
        break;
    case 2:
        paletteOffset = gEwramData->objTileSlots[(s16)graphics].size;
        break;
    }
    if (entity->animFrame < animation->frameCount)
        frame = &animation->frames[entity->animFrame];
    else
        frame = animation->frames;
    count = frame->partCount;
    part = frame->parts;
    origin[0] = frame->originX << 16;
    origin[1] = frame->originY << 16;
    affine = entity->affineIndex;
    gObjAffineGetMatrixFunc(&matrix, affine);
    flipX = gObjAffineGetFlipXFunc(affine);
    flipY = gObjAffineGetFlipYFunc(affine);
    affine <<= 9;
    baseAttributes = ((entity->doubleSize << 1) | 1) << 8;
    baseAttributes |= entity->mosaic << 12;
    baseAttributes |= entity->objMode << 10;
    slot = gObjAllocManyFunc(count);
    if (slot == NULL)
        return 0;
    for (i = 0; i < count; i++, part++, slot += 6)
    {
        {
            output = slot;
            shape = part->shape;
            size = part->size;
            position[0] = part->x << 16;
            position[1] = part->y << 16;
            if (entity->shrinkPartOffsets)
            {
                position[0] = (position[0] >> 8) * 0xE8;
                position[1] = (position[1] >> 8) * 0xE8;
            }
            if (flipX | flipY)
                Mirror(position, position, origin, shape, size, flipX, flipY);
            Rotate(result, position, origin, &matrix, shape, size, entity->doubleSize);
            if (entity->shrinkTransformedOffsets)
            {
                result[0] = (result[0] >> 8) * 0xF6;
                result[1] = (result[1] >> 8) * 0xF6;
            }
            *output++ = (shape << 14) | baseAttributes | (WHOLE(result[1] + entity->posY.whole) & 0xFF);
            *output++ = (size << 14) | affine | (WHOLE(result[0] + entity->posX.whole) & 0x1FF);
            *output = ((entity->palette + (part->paletteOffset & entity->paletteMask)) << 12)
                | gObjTilesGetTileFunc(part->gfxFrame ? graphics + paletteOffset : graphics, part->tileX, part->tileY);
            gObjAddToLayerFunc(slot, entity->spriteLayer);
        }
    }
    return 1;
}

/* Upstream functions 0x08039DC0..0x0803ACDF from the previously unlinked
 * this file, now linked as matching C.
 * Sprite_DrawParts/Sprite_DrawPartsAffine (no upstream C yet) stay in asm/this file.s. */

extern void *gSpriteDrawAffineFunc;
extern void *gSpriteDrawFunc;
extern u8 gSpriteDrawArmCode[];
extern u8 gSpriteDrawAffineArmCode[];

/**
 * @brief 39DC0 | Reset the graphics cache and all sprite slots
 *
 * Also copies the ARM sprite drawers to IWRAM (gSpriteDrawArmCode,
 * gSpriteDrawAffineArmCode) and points gSpriteDrawFunc /
 * gSpriteDrawAffineFunc at them.
 */
void Sprite_Init(void)
{
    s32 i;
    struct SpriteSlot *slot;

    gEwramData->objGfxFreeHead = 0;
    gEwramData->objGfxUsedHead = -1;
    gEwramData->spriteFreeHead = 0;
    gEwramData->spriteUsedHead = -1;
    ObjGfx_Init();

    slot = gEwramData->spriteSlots;
    i = 0;
    // SCREEN_SIZE_X?
    for (; i < 0xF0; i++, slot++)
    {
        slot->prev = i - 1;
        slot->next = i + 1;
        slot->gfxEntry = -1;
        slot->tileSlot = -1;
        slot->sheet = NULL;
    }

    gEwramData->spriteSlots[0].prev = -1;
    gEwramData->spriteSlots[0xEF].next = -1;
    __Obj_InstallCallbacks_from_thumb();
    CPU_COPY_32(Sprite_DrawPartsArm, gSpriteDrawArmCode, 0x400);
    CPU_COPY_32(Sprite_DrawPartsAffineArm, gSpriteDrawAffineArmCode, 0x600);
    gSpriteDrawFunc = gSpriteDrawArmCode;
    gSpriteDrawAffineFunc = gSpriteDrawAffineArmCode;
}

static inline void* TileAddress(s32 offset)
{
    return VRAM_BASE + 0x10000 + offset;
}

static inline s32 IsColor256()
{
    return (gEwramData->objTileFlags & OBJ_TILES_COLOR256) != 0;
}

static inline void CopyTileRows(s32 slot, u8 *src, s32 size, s32 queued)
{
    u16 dst;
    s32 rows;
    u32 rowBytes;
    s32 i;

    dst = ObjTiles_GetSlotTile(slot) << 5;
    rows = size * 4;
    rowBytes = 0x200;
    if (IsColor256() == 1)
    {
        rowBytes <<= 1;
    }
    if (queued == 0)
    {
        for (i = 0; i < rows; i++)
        {
            DMA_COPY_16(3, src, TileAddress(dst), rowBytes);
            src += rowBytes;
            dst += 0x400;
        }
    }
    else
    {
        for (i = 0; i < rows; i++)
        {
            DmaQueue_IndirectCopy(rowBytes, (u32 *) src, TileAddress(dst));
            src += rowBytes;
            dst += 0x400;
        }
    }
}

static inline void LoadTiles(u8 *resource, s32 slot, s32 size, u32 queued)
{
    switch (resource[0])
    {
        case 1:
            ObjGfx_Decompress(resource);

            CopyTileRows(slot, gEwramData->gfxDecompTiles, size, queued);
            break;

        case 0:
            CopyTileRows(slot, resource + 4, size, queued);
            break;
    }
}

static inline void AddEntry(u8 lifetime, u8 size, u8 count, s32 slot, u8 *source, s32 kind)
{
    s32 entry;
    s16 entry16;
    s16 usedHead;
    s32 i;

    if (gEwramData->objGfxFreeHead != -1)
    {
        entry = gEwramData->objGfxFreeHead;
        entry16 = gEwramData->objGfxFreeHead;
        usedHead = gEwramData->objGfxUsedHead;

        gEwramData->objGfxFreeHead = gEwramData->objGfx[entry16].next;
        if (gEwramData->objGfx[entry16].next != -1)
        {
            gEwramData->objGfx[gEwramData->objGfx[entry16].next].prev = -1;
        }
        if (usedHead != -1)
        {
            gEwramData->objGfx[usedHead].prev = entry16;
        }
        gEwramData->objGfxUsedHead = entry16;
        gEwramData->objGfx[entry16].next = usedHead;
        gEwramData->objGfx[entry16].prev = -1;
        gEwramData->objGfx[entry].tileSlot = slot;
        gEwramData->objGfx[entry].kind = kind;
        if (gEwramData->objGfxSaved == 0)
        {
            gEwramData->objGfx[entry].lifetime = lifetime;
        }
        else
        {
            gEwramData->objGfx[entry].lifetime = 2;
        }

        for (i = 0; i < count; i++)
        {
            gEwramData->objGfx[entry].frames[i] = i;
        }
        gEwramData->objGfx[entry].size = size;
        gEwramData->objGfx[entry].count = count;
        gEwramData->objGfx[entry].source = source;
    }
}

static inline s32 SlotsForHeight(s32 height)
{
    s32 size;
    
    switch (height)
    {
        case 16:
            size = 4;
            break;

        case 8:
            size = 2;
            break;

        case 4:
            size = 1;
            break;

        default:
            size = -1;
            break;
    }
    return size;
}

static inline void LoadFrameList(u8 *list, s32 size, s32 count, u32 queued, s32 slot)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        LoadTiles(*(u8**)(list + (i * 4)), slot + (i * size), size, queued & 0xFFFF);
    }
}

/**
 * @brief 39E90 | Load a type 0/1 graphics resource (one frame)
 *
 * Returns the slot of the cached copy if the resource is already loaded.
 * Height 16/8/4 tile rows needs 4/2/1 slots.
 *
 * @param resource Graphics resource
 * @param flags Bits 0-15: nonzero = copy through the DMA queue (in VBlank);
 *              bits 16-31: entry lifetime
 * @return s32 First tile slot, or a negative value if no slots are free
 */
s32 ObjGfx_Load(u8 *resource, s32 flags)
{
    s32 slot;
    s32 size;
    u32 tmp;
    u8 *res;

    res = resource;
    slot = ObjGfx_Find(resource);
    if (slot >= 0)
    {
        return slot;
    }

    size = SlotsForHeight(resource[3]);
    slot = tmp = ObjTiles_Alloc(size, 1, flags);
    if (slot < 0)
    {
        return slot;
    }

    LoadTiles(res, slot, size, flags & 0xFFFF);

    tmp = flags;
    AddEntry(tmp >> 0x10, size, 1, slot, resource, 0);
    return slot;
}

/**
 * @brief 3A1E8 | Load the first frames of a type 2 frame list
 *
 * @param list Frame list resource (frame pointers at +4)
 * @param size Slots per frame
 * @param count Frames to load side by side (frame i into slot + i * size)
 * @param flags As for ObjGfx_Load
 * @return s32 First tile slot, or a negative value if no slots are free
 */
s32 ObjGfx_LoadFrames(u8 *list, s32 size, s32 count, s32 flags)
{
    s32 slot;
    s32 found;
    u32 tmp;

    found = ObjGfx_Find(list);
    if (found >= 0)
    {
        return found;
    }

    slot = tmp = ObjTiles_Alloc(size, count, flags);
    if (slot < 0)
    {
        return slot;
    }

    LoadFrameList(list + 4, size, count, flags, slot);

    tmp = flags;
    AddEntry(tmp >> 0x10, size, count, slot, list, 1);
    return slot;
}

/**
 * @brief 3A544 | Decompress a type 3 frame set and load its first frames
 *
 * Only one set can be loaded at a time (objGfxSet).
 *
 * @param set Frame set resource
 * @param size Slots per frame
 * @param count Frames to load side by side
 * @param flags As for ObjGfx_Load
 * @return s32 First tile slot, or -1 / a negative value on failure
 */
s32 ObjGfx_LoadSet(u8 *set, s32 size, s32 count, s32 flags)
{
    s32 slot;
    s32 found;
    u32 tmp;

    found = ObjGfx_Find(set);
    if (found >= 0)
    {
        return found;
    }

    if (gEwramData->objGfxSet != NULL)
    {
        return -1;
    }

    slot = tmp = ObjTiles_Alloc(size, count, flags);
    if (slot < 0)
    {
        return slot;
    }

    ObjGfx_DecompressSet((struct EwramData_unk19404 *) set);

    LoadFrameList((u8*)gEwramData->objGfxSetFrames, size, count, flags, slot);

    tmp = flags;
    AddEntry(tmp >> 0x10, size, count, slot, set, 2);

    return slot;
}

/**
 * @brief 3A8C8 | Copy slots of a type 0/1 resource into an existing run
 *
 * @param tileSlot First slot of the run
 * @param resource Graphics resource (one slot = 0x800 bytes, 0x1000 in 8bpp)
 * @param offset Destination slot relative to tileSlot
 * @param first Checked together with count against the run's size
 * @param count Number of slots to copy
 * @param flags Bits 0-15: nonzero = copy through the DMA queue
 * @return s32 1, or 0 if first + count exceeds the run
 */
s32 ObjGfx_LoadTilesAt(s32 tileSlot, u8 *resource, s32 offset, s32 first, u32 count, s32 flags)
{
    u8 *tiles;
    s32 slotBytes;
    s32 i;
    s32 size;

    size = gEwramData->objTileSlots[(s16)tileSlot].size;
    if ((first + count) > size)
    {
        return 0;
    }

    slotBytes = 0x800;
    if (IsColor256() == 1)
    {
        slotBytes <<= 1;
    }

    switch (resource[0])
    {
        case 1:
            ObjGfx_Decompress(resource);
            tiles = gEwramData->gfxDecompTiles;
            break;
        case 0:
            tiles = resource + 4;
            break;
    }

    for (i = 0; i < count; i++)
    {
        CopyTileRows(tileSlot + offset + i, tiles + slotBytes * i, 1, flags & 0xFFFF);
    }
    return 1;
}

/**
 * @brief 3AA20 | Show another source frame in one loaded frame of an entry
 *
 * For kind 1 (frame list) and kind 2 (frame set) entries: copies source frame
 * 'frame' into loaded frame 'index' and records it in frames[index].
 *
 * @param tileSlot First tile slot of the entry
 * @param index Loaded frame to replace
 * @param frame Source frame of the list/set
 * @param flags As for ObjGfx_Load
 * @return s32 1, or 0 for a single-resource entry
 */
s32 ObjGfx_SetFrame(s32 tileSlot, s32 index, s32 frame, s32 flags)
{
    s32 entryIndex;
    struct ObjGfxEntry *entry;
    u8 **list;

    entryIndex = ObjGfx_FindEntry(tileSlot);
    entry = &gEwramData->objGfx[entryIndex];
    if (entry->kind == 0)
    {
        return 0;
    }

    if (entry->kind == 1)
    {
        list = entry->source;
    }
    else
    {
        list = (u8**)&gEwramData->objGfxSetList;
    }
    list++;

    ObjGfx_LoadTilesAt(tileSlot, list[frame], index * entry->size, 0, entry->size, flags);
    entry->frames[index] = frame;
    return 1;
}

/* Previous slot in the sprite slot list. */
static inline s32 SpriteSlotPrev(u32 slot)
{
    return gEwramData->spriteSlots[slot].prev;
}

/**
 * @brief 3AAEC | Release the entity's sprite slot
 *
 * @param entity Entity
 * @param freeGfx Nonzero: also unload the graphics (ObjGfx_Release + ObjTiles_Free)
 */
void Sprite_Free(struct EwramData_EntityData *entity, u32 freeGfx)
{
    s32 tileSlot;
    s16 freeHead;
    s16 next;
    s16 prev;
    u32 slot;

    if (entity->spriteSlot == 0xFF)
    {
        return;
    }

    if (freeGfx != 0)
    {
        tileSlot = gEwramData->spriteSlots[entity->spriteSlot].tileSlot;
        ObjGfx_Release(tileSlot);
        ObjTiles_Free(tileSlot);
    }

    slot = entity->spriteSlot;
    if (slot < 0xF0)
    {
        gEwramData->spriteSlots[slot].gfxEntry = -1;
        gEwramData->spriteSlots[slot].tileSlot = -1;
        gEwramData->spriteSlots[slot].sheet = NULL;
        freeHead = gEwramData->spriteFreeHead;
        prev = SpriteSlotPrev(slot);
        next = gEwramData->spriteSlots[slot].next;
        if (slot == gEwramData->spriteUsedHead)
        {
            gEwramData->spriteUsedHead = next;
            if (next != -1)
            {
                gEwramData->spriteSlots[next].prev = -1;
            }
        }
        else
        {
            gEwramData->spriteSlots[prev].next = next;
            if (next != -1)
            {
                gEwramData->spriteSlots[next].prev = prev;
            }
        }

        if (freeHead != -1)
        {
            gEwramData->spriteSlots[freeHead].prev = slot & 0xFF;
        }
        gEwramData->spriteSlots[slot].next = freeHead;
        gEwramData->spriteSlots[slot].prev = -1;
        gEwramData->spriteFreeHead = slot;
    }
    entity->spriteSlot = 0xFF;
    Entity_RemoveFromDrawList(entity);
}

/**
 * @brief 3AC40 | Draw the entity's sprite for this frame
 *
 * Hidden entities count as drawn. drawSkip > 0 draws only every
 * (drawSkip + 1)-th frame. Entities outside the screen (x -64..304,
 * y -32..224) are skipped. The affine drawer is used when entity->affine is set.
 *
 * @param entity Entity
 * @return s32 1 if drawn (or hidden), 0 otherwise
 */
s32 Sprite_Draw(struct EwramData_EntityData *entity)
{
    s32 skip;
    s32 result;

    if (entity->hidden)
    {
        return 1;
    }

    if (entity->spriteSlot >= 0xF0)
    {
        return 0;
    }

    if (entity->drawSkip != 0)
    {
        skip = entity->drawSkipCounter != 0;
        entity->drawSkipCounter += 1;
        if (entity->drawSkipCounter > entity->drawSkip)
        {
            entity->drawSkipCounter = 0;
        }
        if (skip != 0)
        {
            return 0;
        }
    }

    // TODO: unk_526 needs to be s16 for this to work
    // if (entity->posX.part16.integer < -0x40 || entity->posX.part16.integer > 0x130)
    if ((((entity->posX.part16.integer + 0x40) << 0x10) > 0x01700000U))
    {
        return 0;
    }

    if (entity->posY.part16.integer < -0x20)
    {
        return 0;
    }

    if (entity->posY.part16.integer > 0xE0)
    {
        return 0;
    }

    if (entity->affine)
    {
        result = ((s32 (*)(struct EwramData_EntityData *)) gSpriteDrawAffineFunc)(entity);
    }
    else
    {
        result = ((s32 (*)(struct EwramData_EntityData *)) gSpriteDrawFunc)(entity);
    }
    return result;
}

/* Sprite_BuildSingle: single-part variant of Sprite_DrawParts that writes one OAM entry to
 * gEwramData->singleSpriteOam and its offset to singleSpriteX/4; matched. */

/**
 * @brief 3ACE0 | Build the OAM template of a one-part sprite
 *
 * Stores the attributes of the first part of the current frame in
 * singleSpriteOam and its offset in singleSpriteX/Y for Sprite_DrawSingle.
 *
 * @param entity Entity with a sprite
 * @return s32 1 if built, 0 without a sheet
 */
s32 Sprite_BuildSingle(struct EwramData_EntityData *entity)
{
    s32 origin[2];
    s32 position[2];
    s32 graphics;
    u32 baseAttributes;
    s32 paletteOffset = 0;
    struct SpriteSheet *animation;
    struct SpriteFrame *frame;
    struct SpritePart *part;
    u16 *output;
    s32 shape, size;
    s32 flipX, flipY;
    struct EwramData *ew;
    s32 id;

    if (entity->spriteSlot >= 0xF0)
        return 0;
    ew = gEwramData;
    id = entity->spriteSlot;
    graphics = ew->spriteSlots[id].tileSlot;
    animation = ew->spriteSlots[(u8)id].sheet;
    if (animation == NULL)
        return 0;
    switch (entity->animMode)
    {
    case 0:
    case 1:
        break;
    case 2:
        paletteOffset = ew->objTileSlots[(s16)graphics].size;
        break;
    }
    if (entity->animFrame < animation->frameCount)
        frame = &animation->frames[entity->animFrame];
    else
        frame = animation->frames;
    part = frame->parts;
    origin[0] = frame->originX << 16;
    origin[1] = frame->originY << 16;
    baseAttributes = (entity->mosaic << 12) | (entity->objMode << 10);
    output = gEwramData->singleSpriteOam;
    shape = part->shape;
    size = part->size;
    flipX = entity->flipX;
    flipY = entity->flipY;
    position[0] = part->x << 16;
    position[1] = part->y << 16;
    if (flipX | flipY)
        Mirror(position, position, origin, shape, size, flipX, flipY);
    gEwramData->singleSpriteX = position[0];
    gEwramData->singleSpriteY = position[1];
    *output++ = (shape << 14) | baseAttributes | (WHOLE(position[1] + entity->posY.whole) & 0xFF);
    *output++ = (size << 14) | ((flipY ^ (part->flip & 1)) << 13) | (((part->flip & 2) ? flipX ^ 1 : flipX) << 12) | (WHOLE(position[0] + entity->posX.whole) & 0x1FF);
    *output = ((entity->palette + (part->paletteOffset & entity->paletteMask)) << 12)
        | ObjTiles_GetTile(part->gfxFrame ? graphics + paletteOffset : graphics, part->tileX, part->tileY);
    return 1;
}

/* Upstream functions 0x0803AEFC..0x0803B183 from the previously unlinked
 * this file, now linked as matching C.
 * Sprite_BuildSingle and ObjGfx_RestoreState (no upstream C yet) stay in asm. Hard-coded
 * ROM addresses of the draft were replaced by data labels. */

struct SingleSpriteOamView {
    u8 attribute0Low;
    u8 attribute0High;
    s16 attribute1;
};

/**
 * @brief 3AEFC | Draw the entity as one OAM sprite at its screen position
 *
 * @param entity Entity
 * @return s32 1 if drawn, 0 if off screen or no OAM entry is free
 */
s32 Sprite_DrawSingle(struct EwramData_EntityData *entity)
{
    s16 screenY;
    s32 y;
    s32 x;
    s32 row;
    u32 column;
    u64 *oam;
    struct SingleSpriteOamView *attr;

    if ((u32)((entity->posX.part16.integer + 0x40) << 0x10) > 0x01700000U)
        return 0;

    screenY = entity->posY.part16.integer;
    if (screenY < -0x20)
        return 0;
    if (screenY > 0xE0)
        return 0;

    oam = (void *)Obj_Alloc();
    if (oam == NULL)
        return 0;

    attr = (struct SingleSpriteOamView *)oam;
    {
        struct EwramData *ewram = gEwramData;

        *oam = *(u64 *)ewram->singleSpriteOam;
        y = ewram->singleSpriteY + entity->posY.whole;
    }
    if (y < 0)
        row = -(-y >> 0x10);
    else
        row = (u32)y >> 0x10;
    attr->attribute0Low = row;

    x = gEwramData->singleSpriteX + entity->posX.whole;
    if (x < 0)
        column = -(-x >> 0x10) & 0x1FF;
    else
        column = ((u32)x >> 0x10) & 0x1FF;
    attr->attribute1 = (attr->attribute1 & ~0x1FF) | column;

    Obj_AddToLayer((void *)oam, entity->spriteLayer);
    return 1;
}

/**
 * @brief 3AFB8 | Find the tile slot of an already loaded resource
 *
 * sCommonObjectGfx is replaced by its language 2/3 variant first.
 *
 * @param resource Graphics resource
 * @return s32 First tile slot, or -1 if not loaded
 */
s32 ObjGfx_Find(u8 *resource)
{
    s32 entry;

    entry = gEwramData->objGfxUsedHead;
    if (resource == sCommonObjectGfx)
    {
        switch (gEwramData->gameState.language)
        {
            case 2:
                resource = sCommonObjectGfxLanguage2;
                break;
            case 3:
                resource = sCommonObjectGfxLanguage3;
                break;
        }
    }

    while (entry != -1)
    {
        if (gEwramData->objGfx[entry].source == resource)
        {
            return gEwramData->objGfx[entry].tileSlot;
        }
        entry = gEwramData->objGfx[entry].next;
    }
    return -1;
}

/**
 * @brief 3B04C | Back up the graphics cache and drop entries with lifetime 0
 *
 * Used before menus/cutscenes; the dropped entries are reloaded by
 * ObjGfx_RestoreState.
 *
 * @return s32 1, or 0 if a backup already exists
 */
s32 ObjGfx_SaveState(void)
{
    s32 i;

    if (gEwramData->objGfxSaved == 0)
    {
        DMA_COPY_32(3, &gEwramData->objGfx, &gEwramData->objGfxBackup, 0x140);
        DMA_COPY_32(3, &gEwramData->objGfxFreeHead, &gEwramData->objGfxListBackup, 0x8);
        DMA_COPY_32(3, &gEwramData->objGfxSetList, &gEwramData->objGfxSetListBackup, 0x1C);
        gEwramData->objGfxSetBackup = gEwramData->objGfxSet;
        gEwramData->objGfxSet = 0;

        for (i = 0; i < 0x10; i++)
        {
            if (gEwramData->objGfx[i].tileSlot != -1)
            {
                if (gEwramData->objGfx[i].lifetime == 0)
                {
                    ObjTiles_Free(gEwramData->objGfx[i].tileSlot);
                    gEwramData->objGfx[i].tileSlot = -1;
                    gEwramData->objGfx[i].kind = 0;
                    gEwramData->objGfx[i].lifetime = 0;
                    gEwramData->objGfx[i].frames[0] = 0;
                    gEwramData->objGfx[i].frames[1] = 0;
                    gEwramData->objGfx[i].source = 0;
                }
            }
        }

        gEwramData->objGfxSaved = 1;
        return 1;
    }
    return 0;
}

/* ObjGfx_RestoreState: matched (row-copy inlines as in this file,
   return 0 at the end, i = 0 before slot, separate list variables for case 1/2). */
/* Reload all sprite graphics slots into VRAM after the slot table was saved. */

static inline s32 IsColor256_2(void)
{
    return (gEwramData->objTileFlags & OBJ_TILES_COLOR256) != 0;
}

static inline void CopyRows(s32 tile, u8 *src, s32 width, s32 queued)
{
    u16 dst;
    s32 n;
    u32 size;
    s32 k;

    dst = ObjTiles_GetSlotTile(tile) << 5;
    n = width * 4;
    size = 0x200;
    if (IsColor256_2() == 1)
    {
        size <<= 1;
    }
    if (queued == 0)
    {
        for (k = 0; k < n; k++)
        {
            DMA_COPY_16(3, src, TileAddress(dst), size);
            src += size;
            dst += 0x400;
        }
    }
    else
    {
        for (k = 0; k < n; k++)
        {
            DmaQueue_IndirectCopy(size, (u32 *)src, TileAddress(dst));
            src += size;
            dst += 0x400;
        }
    }
}

static inline void LoadTiles2(u8 *data, s32 tile, s32 width, u32 queued)
{
    switch (data[0])
    {
        case 1:
            ObjGfx_Decompress(data);
            CopyRows(tile, gEwramData->gfxDecompTiles, width, queued);
            break;

        case 0:
            CopyRows(tile, data + 4, width, queued);
            break;
    }
}

s32 ObjGfx_RestoreState(void)
{
    s32 i;
    s32 j;
    s32 tile;
    s32 width;
    u8 **list;
    u8 **sharedList;
    struct ObjGfxEntry *entry;

    if (gEwramData->objGfxSaved != 0)
    {
    entry = gEwramData->objGfx;
    for (i = 0; i < 16; i++, entry++)
    {
        if (entry->tileSlot != -1 && entry->lifetime == 2)
            ObjTiles_Free(entry->tileSlot);
    }
    DMA_COPY_32(3, (u8 *)gEwramData + 0x11078, (u8 *)gEwramData + 0x11070, 8);
    DMA_COPY_32(3, (u8 *)gEwramData + 0x10008, (u8 *)gEwramData + 0xFEC4, 0x140);
    DMA_COPY_32(3, (u8 *)gEwramData + 0x10154, (u8 *)gEwramData + 0x25468, 0x1C);
    gEwramData->objGfxSet = NULL;
    if (gEwramData->objGfxSetBackup != NULL)
        ObjGfx_DecompressSet(gEwramData->objGfxSetBackup);
    for (i = 0, entry = gEwramData->objGfx; i < 16; i++, entry++)
    {
        if (entry->tileSlot == -1)
            continue;
        if (entry->lifetime != 0)
            continue;
        width = entry->size;
        tile = ObjTiles_AllocAt(entry->tileSlot, width, entry->count);
        switch (entry->kind)
        {
        case 0:
            LoadTiles2(entry->source, tile, width, 0);
            break;
        case 1:
            list = (u8 **)((u8 *)entry->source + 4);
            for (j = 0; j < entry->count; j++)
                LoadTiles2(list[entry->frames[j]], tile + width * j, width, 0);
            break;
        case 2:
            sharedList = (u8 **)gEwramData->objGfxSetFrames;
            for (j = 0; j < entry->count; j++)
                LoadTiles2(sharedList[entry->frames[j]], tile + width * j, width, 0);
            break;
        }
    }
    gEwramData->objGfxSaved = 0;
    return 1;
    }
    return 0;
}

/* Upstream functions 0x0803B66C..0x0803DB6B from the previously unlinked
 * this file, now linked; split at 0x0803C294 and 0x0803C49C.
 * Hard-coded ROM addresses of the draft were replaced by data
 * labels sSpriteSheetDefault1/80/90. */

/**
 * @brief Copy the tiles of one animation frame (decompressing LZ77 data when
 * needed) into an object tile slot; return 0 when it does not fit.
 */
s32 ObjTiles_LoadFrame(s32 firstSlot, struct ObjTileFrameSource *src, u32 variant, s32 tileOffset, u32 count, s32 useDmaQueue)
{
    u8 *tiles;
    s32 frameSize;
    s32 j;
    s32 baseTile;
    u16 vramOffset;
    u32 i;
    u8 *source;
    s32 one;
    s32 halfSize;
    s32 srcOffset;
    s32 slot;
    u8 slotSize;
    s32 rows;
    s32 halfwords;
    u8 *tmp;

    one = 1;
    slotSize = gEwramData->objTileSlots[(s16)firstSlot].size;
    if ((tileOffset + count) > slotSize)
    {
        return 0;
    }

    frameSize = 0x200;
    if ((((u16) gEwramData->objTileFlags >> 1) & one) == 1)
    {
        frameSize *= 2;
    }
    frameSize *= 4;

    if (src->kind == 1)
    {
        u8 *tmp = gEwramData->gfxDecompBuffer;
        LZ77UnCompWram(src->compressedData, tmp);
        tiles = gEwramData->gfxDecompTiles;
    }
    else
    {
        tiles = (u8*)&src->compressedData;
    }
    tiles += (((variant >> 1) & 1) << 0xC) + ((variant & 1) << 8);

    for (i = 0; i < count; i++)
    {
        halfwords = 0x80;
        rows = 4;
        halfSize = 0x100;

        srcOffset = frameSize * i;
        slot = firstSlot + i;
        source = tiles + srcOffset;
        vramOffset = ObjTiles_GetSlotTile(slot) << 5;

        baseTile = vramOffset;
        for (j = 0; j < rows; j++)
        {
            if (useDmaQueue != 0)
            {
                DmaQueue_IndirectCopy(halfSize, (u32 *) source, VRAM_BASE + 0x10000 + vramOffset);
            }
            else
            {
                DMA_COPY_16(3, source, VRAM_BASE + 0x10000 + vramOffset, halfwords * 2);
            }
            source += 0x200;
            vramOffset += 0x400;
        }
        // sub_0803B66C_inline_0(source, vramOffset, rows, useDmaQueue, halfSize);

        vramOffset = baseTile + halfSize;
        for (j = 0; j < rows; j++)
        {
            if (useDmaQueue != 0)
            {
                DmaQueue_IndirectCopy(halfSize, (u32 *) source, VRAM_BASE + 0x10000 + vramOffset);
            }
            else
            {
                DMA_COPY_16(3, source, VRAM_BASE + 0x10000 + vramOffset, halfwords * 2);
            }
            source += 0x200;
            vramOffset += 0x400;
        }
        // sub_0803B66C_inline_0(source, vramOffset + halfSize, rows, useDmaQueue, halfSize);

        // sub_0803B66C_inline_1(firstSlot + i, useDmaQueue, tiles + (frameSize * i), rows, halfSize);
    }
    return 1;
}

/**
 * @brief 3B800 | Load a graphics resource of any type
 *
 * @param resource Graphics resource (type in byte 0, see ObjGfxEntry)
 * @param flags As for ObjGfx_Load
 * @return s32 First tile slot, or a negative value on failure
 */
s32 ObjGfx_LoadResource(u8 *resource, s32 flags)
{
    s32 slot;

    slot = -1;

    switch (resource[0])
    {
        case 0:
        case 1:
            slot = ObjGfx_Load(resource, flags);
            break;
        case 2:
            slot = ObjGfx_LoadFrames(resource, resource[2], resource[3], flags);
            break;
        case 3:
            slot = ObjGfx_LoadSet(resource, resource[2], resource[3], flags);
            break;
    }

    return slot;
}

/**
 * @brief 3B840 | Get the source frame shown in a loaded frame of an entry
 *
 * @param tileSlot First tile slot of the entry
 * @param index Loaded frame
 * @return s32 Source frame index, 0 for a single-resource entry
 */
s32 ObjGfx_GetFrame(s32 tileSlot, s32 index)
{
    s32 entryIndex;
    s32 unused;
    struct ObjGfxEntry *entry;

    entryIndex = ObjGfx_FindEntry(tileSlot);

    entry = &gEwramData->objGfx[entryIndex];
    if (entry->kind == 0)
    {
        return 0;
    }
    return entry->frames[index];
}

/**
 * @brief 3B8C4 | Give an entity a sprite with the default one-frame sheet
 *
 * The sheet is chosen by the size of the tile slot run (1, 2 or 4 slots).
 *
 * @param entity Entity without a sprite
 * @param tileSlot First tile slot of the loaded graphics
 * @param palette OBJ palette
 * @return s32 1 if attached
 */
s32 Sprite_AttachDefault(struct EwramData_EntityData *entity, s32 tileSlot, s32 palette)
{
    s32 result;
    u8 size;

    size = gEwramData->objTileSlots[(s16)tileSlot].size;
    switch (size)
    {
        case 4:
            result = Sprite_Attach(entity, (void *)sSpriteSheetDefault4, tileSlot, palette);
            break;

        case 2:
            result = Sprite_Attach(entity, (void *)sSpriteSheetDefault2, tileSlot, palette);
            break;

        case 1:
            result = Sprite_Attach(entity, (void *)sSpriteSheetDefault1, tileSlot, palette);
            break;
    }

    // BUG: returns uninitialized value if none of the cases are met
    return result;
}

/**
 * @brief 3B924 | Give an entity a sprite slot
 *
 * @param entity Entity without a sprite (spriteSlot 0xFF)
 * @param sheet Sprite sheet
 * @param tileSlot First tile slot of the loaded graphics
 * @param palette OBJ palette
 * @return s32 1 if attached, 0 if the entity has a sprite or no slot is free
 */
s32 Sprite_Attach(struct EwramData_EntityData *entity, void *sheet, s32 tileSlot, u32 palette)
{
    s32 slot;

    if (entity->spriteSlot != 0xFF)
    {
        return 0;
    }

    slot = Sprite_AllocSlot(sheet, tileSlot);
    if (slot != 0xFF)
    {
        entity->spriteSlot = slot;
        entity->palette = palette & 0xFF;
        entity->animFrame = 0;
        Entity_AddToDrawList(entity);
        return 1;
    }

    return 0;
}

/**
 * @brief 3B980 | Remove a cache entry and free its tile slots
 *
 * @param tileSlot First tile slot of the entry
 */
void ObjGfx_Unload(s32 tileSlot)
{
    ObjGfx_Release(tileSlot);
    ObjTiles_Free(tileSlot);
}

/**
 * @brief 3B998 | Get the tile slot of the entity's graphics
 *
 * @param entity Entity with a sprite
 * @return s32 First tile slot
 */
s32 Sprite_GetTileSlot(struct EwramData_EntityData *entity)
{
    return gEwramData->spriteSlots[entity->spriteSlot].tileSlot;
}

/**
 * @brief 3B9B4 | LZ77-decompress a type 1 resource into gfxDecompBuffer
 *
 * @param resource Type 1 graphics resource (data pointer at +4)
 */
void ObjGfx_Decompress(void *resource)
{
    void *dst;

    dst = gEwramData->gfxDecompBuffer;
    LZ77UnCompWram((void*)READ_32(resource + 4), dst);
}

/**
 * @brief 3B9D0 | Entity draw callback: Sprite_Draw
 *
 * @param entity Entity
 */
void Sprite_DrawEntity(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
}

/**
 * @brief 3B9DC | Restore every backed-up base palette of a buffer from 'saved'
 *
 * @param buffer objPalette or bgPalette
 * @return s32 1 if anything was restored, 0 if no backup exists
 */
s32 Palette_RestoreSaved(struct PaletteBuffer *buffer)
{
    s32 slot;
    s32 mask;
    s32 bit;

    if (buffer->savedMask == 0xFFFF)
    {
        DMA_COPY_16(3, &buffer->saved, &buffer->base, 0x200);
        buffer->savedMask = 0;
        buffer->uploaded = 0;
        return 1;
    }

    if (buffer->savedMask != 0)
    {
        mask = buffer->savedMask;
        bit = 1;
        for (slot = 0; slot < 0x10; slot++)
        {
            if (mask & bit)
            {
                DMA_COPY_16(3, &buffer->saved[slot], &buffer->base[slot], 0x20);
            }
            bit <<= 1;
        }
        buffer->savedMask = 0;
        buffer->uploaded = 0;
        return 1;
    }

    return 0;
}

/**
 * @brief 3BA88 | Compute the output palettes with the per-palette blends
 *
 * Palettes with an active blend slot are mixed with their target palette
 * (base * weight + target * (1 - weight)); the others are copied unchanged.
 *
 * @param buffer objPalette or bgPalette
 */
void Palette_BlendSlots(struct PaletteBuffer *buffer)
{
    s32 color;
    struct PaletteFade *fade;
    s32 activeMask;
    s16 *dst;
    u32 weight;
    u32 invWeight;
    s32 slot;
    struct PaletteFade *fade_2;
    u16 *src;
    s32 r;
    s32 g;
    s32 b;
    s32 srcR;
    s32 srcG;
    s32 srcB;
    s32 tgtR;
    s32 tgtG;
    s32 tgtB;
    u16 *target;

    fade_2 = &buffer->fade;
    fade = fade_2;
    activeMask = fade_2->activeMask;

    for (slot = 0; slot < 0x10; slot++)
    {
        src = buffer->base[slot];
        dst = buffer->output[slot];
        if ((activeMask >> slot) & 1)
        {
            target = fade->target[slot];
            weight = fade->weight[slot];
            invWeight = 0x10000 - weight;
            buffer->uploaded = 0;

            for (color = 0; color < 0x10; color++)
            {
                srcR = src[color] & 0x1F;
                srcG = src[color] & (0x1F << 5);
                srcB = src[color] & (0x1F << 10);

                tgtR = target[color] & 0x1F;
                tgtG = target[color] & (0x1F << 5);
                tgtB = target[color] & (0x1F << 10);

                r = ((srcR * weight) + (tgtR * invWeight)) >> 0x10;
                g = ((srcG * weight) + (tgtG * invWeight)) >> 0x10;
                b = ((srcB * weight) + (tgtB * invWeight)) >> 0x10;
                dst[color] = (r & 0x1F) | (g & (0x1F << 5)) | (b & (0x1F << 10));
            }
        }
        else
        {
            DMA_COPY_16(3, src, dst, 0x20);
        }
    }
}

/**
 * @brief 3BBA4 | Compute the output palettes with the blends and the color fade
 *
 * Like Palette_BlendSlots, then every palette is additionally mixed with the
 * solid fade color (colorR/G/B) by weight[PALETTE_FADE_COLOR].
 *
 * @param buffer objPalette or bgPalette
 */
void Palette_BlendSlotsAndColor(struct PaletteBuffer *buffer)
{
    struct PaletteFade *fade;
    s32 colorR;
    s32 colorG;
    s32 colorB;
    s32 color;
    s32 invWeight;
    s16 *dst;
    u32 colorWeight;
    s32 colorInvWeight;
    s32 slot;
    u16 *src;
    u16 *target;
    u32 weight;
    s32 r;
    s32 g;
    s32 b;
    s32 srcR;
    s32 srcG;
    s32 srcB;
    s32 tgtR;
    s32 tgtG;
    s32 tgtB;
    s32 r_2;
    s32 g_2;
    s32 b_2;
    s32 srcR_2;
    s32 srcG_2;
    s32 srcB_2;

    fade = &buffer->fade;
    colorWeight = fade->weight[PALETTE_FADE_COLOR];
    colorInvWeight = 0x10000 - colorWeight;
    colorR = buffer->fade.colorR * colorInvWeight;
    colorG = (buffer->fade.colorG * colorInvWeight) << 5;
    colorB = (buffer->fade.colorB * colorInvWeight) << 10;
    buffer->uploaded = 0;

    for (slot = 0; slot < 0x10; slot++)
    {
        src = buffer->base[slot];
        dst = buffer->output[slot];
        if (fade->activeMask & (1 << slot))
        {
            target = fade->target[slot];
            weight = fade->weight[slot];
            invWeight = 0x10000 - weight;

            for (color = 0; color < 0x10; color++)
            {
                srcR = src[color] & 0x1F;
                srcG = src[color] & (0x1F << 5);
                srcB = src[color] & (0x1F << 10);

                tgtR = target[color] & 0x1F;
                tgtG = target[color] & (0x1F << 5);
                tgtB = target[color] & (0x1F << 10);

                r = ((srcR * weight) + (tgtR * invWeight)) >> 0x10;
                r = ((r * colorWeight) + colorR) >> 0x10;

                g = ((srcG * weight) + (tgtG * invWeight)) >> 0x10;
                g = ((g * colorWeight) + colorG) >> 0x10;

                b = ((srcB * weight) + (tgtB * invWeight)) >> 0x10;
                b = ((b * colorWeight) + colorB) >> 0x10;

                dst[color] = (r & 0x1F) | (g & (0x1F << 5)) | (b & (0x1F << 10));
            }
        }
        else
        {
            for (color = 0; color < 0x10; color++)
            {
                srcR_2 = src[color] & 0x1F;
                srcG_2 = src[color] & (0x1F << 5);
                srcB_2 = src[color] & (0x1F << 10);

                r_2 = ((srcR_2 * colorWeight) + (colorR)) >> 0x10;
                g_2 = ((srcG_2 * colorWeight) + (colorG)) >> 0x10;
                b_2 = ((srcB_2 * colorWeight) + (colorB)) >> 0x10;

                dst[color] = (r_2 & 0x1F) | (g_2 & (0x1F << 5)) | (b_2 & (0x1F << 10));
            }
        }
    }
}

/**
 * @brief 3BD5C | Advance the weights of all running fades of a buffer
 *
 * Slots that are active, running and pending move by 'step'; a slot that
 * reaches targetWeight is clamped and leaves pendingMask. Does nothing while
 * the game is paused (paused).
 *
 * @param buffer objPalette or bgPalette
 */
void Palette_StepFades(struct PaletteBuffer *buffer)
{
    s32 bit;
    s32 slot;
    struct PaletteFade *fade;
    s32 mask;

    if (gEwramData->paused)
        return;

    fade = &buffer->fade;
    mask = fade->activeMask & fade->runningMask & fade->pendingMask;

    for (slot = 0; slot <= PALETTE_FADE_COLOR; slot++)
    {
        bit = 1 << slot;
        if (mask & bit)
        {
            fade->weight[slot] += fade->step[slot];
            if (fade->step[slot] > 0)
            {
                if (fade->weight[slot] > fade->targetWeight[slot])
                {
                    fade->weight[slot] = fade->targetWeight[slot];
                    fade->pendingMask &= ~bit;
                }
            }
            else
            {
                if (fade->weight[slot] < fade->targetWeight[slot])
                {
                    fade->weight[slot] = fade->targetWeight[slot];
                    fade->pendingMask &= ~bit;
                }
            }
        }
    }
}

/**
 * @brief 3BE04 | Compute the output palettes of both buffers
 *
 * Without active fades the base palettes are copied to output; otherwise
 * the fades are stepped and blended.
 */
void Palette_UpdateFades(void)
{
    s32 activeMask;
    s32 activeMask_2;

    activeMask = gEwramData->objPalette.fade.activeMask;
    if (activeMask == 0)
    {
        DMA_COPY_16(3, &gEwramData->objPalette.base, &gEwramData->objPalette.output, 0x200);
    }
    else
    {
        Palette_StepFades(&gEwramData->objPalette);
        if (activeMask & 0x10000)
        {
            Palette_BlendSlotsAndColor(&gEwramData->objPalette);
        }
        else
        {
            Palette_BlendSlots(&gEwramData->objPalette);
        }
    }

    activeMask_2 = gEwramData->bgPalette.fade.activeMask;
    if (activeMask_2 == 0)
    {
        DMA_COPY_16(3, &gEwramData->bgPalette.base, &gEwramData->bgPalette.output, 0x200);
    }
    else
    {
        Palette_StepFades(&gEwramData->bgPalette);
        if (activeMask_2 & 0x10000)
        {
            Palette_BlendSlotsAndColor(&gEwramData->bgPalette);
        }
        else
        {
            Palette_BlendSlots(&gEwramData->bgPalette);
        }
    }
}

/**
 * @brief 3BEEC | Clear both palette buffers, palette animations and the invert effect
 */
void Palette_Init(void)
{
    DMA_FILL_16(3, 0, &gEwramData->objPalette, 0x840);
    DMA_FILL_16(3, 0, &gEwramData->bgPalette, 0x840);
    gEwramData->objPalette.uploaded = 1;
    gEwramData->bgPalette.uploaded = 1;
    PaletteAnim_Init();
    gEwramData->paletteInvert = 0;
}

/**
 * @brief 3BF60 | Upload changed output palettes to palette RAM (called in VBlank)
 */
void Palette_TransferToPalram(void)
{
    if (gEwramData->objPalette.uploaded == 0)
    {
        DMA_COPY_16(3, &gEwramData->objPalette.output, PALRAM_OBJ, PAL_SIZE);
        gEwramData->objPalette.uploaded = 1;
    }
    gEwramData->objPalette.dirtyMask = 0;

    if (gEwramData->bgPalette.uploaded == 0)
    {
        DMA_COPY_16(3, &gEwramData->bgPalette.output, PALRAM_BASE, PAL_SIZE);
        gEwramData->bgPalette.uploaded = 1;
    }
    gEwramData->bgPalette.dirtyMask = 0;
}

/**
 * @brief 3BFF0 | Back up and stop one palette animation
 *
 * @param slot Palette (0-15)
 * @return s32 1 if saved, 0 if the slot already holds a backup
 */
s32 PaletteAnim_SaveSlot(u16 slot)
{
    struct PaletteAnimState *state;
    s32 bit;

    state = &gEwramData->paletteAnim;
    bit = 1 << slot;
    if ((state->savedMask & bit) != 0)
        return 0;

    DMA_COPY_16(3, &gEwramData->paletteAnim.anims[slot], &gEwramData->paletteAnim.savedAnims[slot], 0xC);
    DMA_FILL_16(3, 0, &gEwramData->paletteAnim.anims[slot], 0xC);
    gEwramData->paletteAnim.savedMask |= bit;
    return 1;
}

/**
 * @brief 3C06C | Restore every backed-up palette animation
 *
 * @return s32 1 if anything was restored, 0 if no backup exists
 */
s32 PaletteAnim_RestoreSaved(void)
{
    s32 mask;
    s32 slot;
    s32 bit;

    if ((gEwramData->paletteAnim.savedMask & 0xFFFF) == 0xFFFF)
    {
        DMA_COPY_16(3, &gEwramData->paletteAnim.savedAnims, &gEwramData->paletteAnim.anims, 0xC0);
        gEwramData->paletteAnim.savedMask &= ~0xFFFF;
        return 1;
    }

    if ((gEwramData->paletteAnim.savedMask & 0xFFFF) != 0)
    {
        mask = gEwramData->paletteAnim.savedMask;
        bit = 1;
        for (slot = 0; slot < 0x10; slot++)
        {
            if (mask & bit)
            {
                DMA_COPY_16(3, &gEwramData->paletteAnim.savedAnims[slot], &gEwramData->paletteAnim.anims[slot], 0xC);
            }
            bit <<= 1;
        }

        gEwramData->paletteAnim.savedMask &= ~0xFFFF;
        return 1;
    }
    return 0;
}

static inline void MarkObjPaletteDirty(s32 slot)
{
    gEwramData->objPalette.dirtyMask |= 1 << slot;
}

/**
 * @brief 3C150 | Advance all palette animations by one frame
 *
 * On the first frame of each animation step the step's palette is copied
 * from the resource into the animation's objPalette slot (same clamping as
 * ObjPalette_LoadRange with count 1).
 */
void PaletteAnim_Update(void)
{
    u8 *step;
    s32 unused;
    u8 *palette;
    s32 n;
    struct PaletteAnimSlot *anim;
    s32 slot_2;
    s32 first;
    s32 slot;
    s64 tmp;

    if (gEwramData->paletteAnimSkipFrame)
    {
        gEwramData->paletteAnimSkipFrame = 0;
        return;
    }

    for (slot = 0; slot < 0x10; slot++)
    {
        anim = &gEwramData->paletteAnim.anims[slot];
        palette = anim->palette;
        if (palette == 0)
            continue;

        if (anim->script == NULL)
            continue;

        /* Current step: 4-byte entries after the u16 step count at +0. */
        step = anim->script + ((anim->frame * 4) + 4);
        if (anim->timer == 0)
        {
            first = step[0];
            tmp = (u16)slot; // needed to match
            slot_2 = (u16)slot;
            n = 1;
            if ((((s32) (slot_2 + 1) <= 0x10)) || (n = 0x10 - slot_2, (n != 0)))
            {
                if ((((first + n) <= palette[2])) || (n = palette[2] - first, (n != 0)))
                {
                    if (n != 0)
                    {
                        DMA_COPY_16(3, palette + 4 + first * 0x20, &gEwramData->objPalette.base[slot_2], n * 0x20);
                        gEwramData->objPalette.uploaded = 0;
                    }

                    for (first = slot_2; first < (slot_2 + n); first++)
                    {
                        MarkObjPaletteDirty(first);
                    }
                }
            }
        }

        anim->timer += 1;
        if (anim->timer >= step[1])
        {
            anim->timer = 0;
            anim->frame += 1;
            if (anim->frame >= *(u16*)anim->script)
            {
                anim->frame = 0;
            }
        }
    }
}
