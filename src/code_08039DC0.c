/* Upstream functions 0x08039DC0..0x0803ACDF from the previously unlinked
 * src/code/code_08039340.c, now linked as matching C.
 * Sprite_DrawParts/Sprite_DrawPartsAffine (no upstream C yet) stay in asm/code/code_08039820.s. */
#include "structs/render_slots.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080E032C.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "code/code_08039340_inline.h"

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

