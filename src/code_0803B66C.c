/* Upstream functions 0x0803B66C..0x0803DB6B from the previously unlinked
 * src/code/code_08039340.c, now linked; split at 0x0803C294 and 0x0803C49C.
 * Hard-coded ROM addresses of the draft were replaced by data
 * labels sSpriteSheetDefault1/80/90. */
#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "structs/render_slots.h"
#include "gba.h"
#include "syscalls.h"
#include "macros.h"
#include "code_08001194.h"

extern const u8 sSpriteSheetDefault1[];
extern const u8 sSpriteSheetDefault2[];
extern const u8 sSpriteSheetDefault4[];

#include "code/code_08039340_inline.h"

/**
 * @brief Copy the tiles of one animation frame (decompressing LZ77 data when
 * needed) into an object tile slot; return 0 when it does not fit.
 */
s32 ObjTiles_LoadFrame(s32 param_0, struct ObjTileFrameSource *src, u32 variant, s32 tileOffset, u32 count, s32 param_5)
{
    u8 *sp4;
    s32 sp8;
    s32 j;
    s32 baseTile;
    u16 var_r4;
    u32 i;
    u8 *var_r6;
    s32 var_0;
    s32 var_1;
    s32 srcOffset;
    s32 slot;
    u8 slotSize;
    s32 var_5;
    s32 halfwords;
    u8 *tmp;

    var_0 = 1;
    slotSize = gEwramData->objTileSlots[(s16)param_0].size;
    if ((tileOffset + count) > slotSize)
    {
        return 0;
    }

    sp8 = 0x200;
    if ((((u16) gEwramData->objTileFlags >> 1) & var_0) == 1)
    {
        sp8 *= 2;
    }
    sp8 *= 4;

    if (src->kind == 1)
    {
        u8 *tmp = gEwramData->gfxDecompBuffer;
        LZ77UnCompWram(src->compressedData, tmp);
        sp4 = gEwramData->gfxDecompTiles;
    }
    else
    {
        sp4 = (u8*)&src->compressedData;
    }
    sp4 += (((variant >> 1) & 1) << 0xC) + ((variant & 1) << 8);

    for (i = 0; i < count; i++)
    {
        halfwords = 0x80;
        var_5 = 4;
        var_1 = 0x100;

        srcOffset = sp8 * i;
        slot = param_0 + i;
        var_r6 = sp4 + srcOffset;
        var_r4 = ObjTiles_GetSlotTile(slot) << 5;

        baseTile = var_r4;
        for (j = 0; j < var_5; j++)
        {
            if (param_5 != 0)
            {
                DmaQueue_IndirectCopy(var_1, (u32 *) var_r6, VRAM_BASE + 0x10000 + var_r4);
            }
            else
            {
                DMA_COPY_16(3, var_r6, VRAM_BASE + 0x10000 + var_r4, halfwords * 2);
            }
            var_r6 += 0x200;
            var_r4 += 0x400;
        }
        // sub_0803B66C_inline_0(var_r6, var_r4, var_5, param_5, var_1);

        var_r4 = baseTile + var_1;
        for (j = 0; j < var_5; j++)
        {
            if (param_5 != 0)
            {
                DmaQueue_IndirectCopy(var_1, (u32 *) var_r6, VRAM_BASE + 0x10000 + var_r4);
            }
            else
            {
                DMA_COPY_16(3, var_r6, VRAM_BASE + 0x10000 + var_r4, halfwords * 2);
            }
            var_r6 += 0x200;
            var_r4 += 0x400;
        }
        // sub_0803B66C_inline_0(var_r6, var_r4 + var_1, var_5, param_5, var_1);

        // sub_0803B66C_inline_1(param_0 + i, param_5, sp4 + (sp8 * i), var_5, var_1);
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
    s32 var_r3;
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
    s32 temp_r0_3;
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
