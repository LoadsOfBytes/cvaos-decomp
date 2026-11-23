/* Upstream functions 0x0803C49C..0x0803DB6B from engine/obj_gfx.c (split off
 * engine/obj_gfx.c).
 * Palette system part 2: invert/tint effects, palette loading, save/restore,
 * palette animation control and fade control (see the PaletteBuffer comment
 * in structs/ewram.h), followed by the OBJ tile slot allocator. */
#include "structs/render_slots.h"
#include "gba.h"

#include "graphics_inline.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "palette.h"

/**
 * @brief 3C49C | Apply the invert effect to the output palettes
 *
 * Moves colors 1-15 of BG palettes 0-9 and OBJ palettes 10-13 toward their
 * inverse (31 - c) by the 16.16 strength paletteInvert. A strength of 1
 * turns the effect off; the effect is one-shot and paletteInvert is cleared
 * after it has been applied.
 */
void Palette_ApplyInvert(void)
{
    struct PaletteBuffer *buffer;
    u32 r;
    s32 r_2;
    s32 rWeighted;
    s32 gWeighted;
    s32 bWeighted;
    s32 rWeighted_2;
    s32 gWeighted_2;
    s32 bWeighted_2;
    s32 pal;
    u16 *dst;
    u16 *dst_2;
    u16 *src;
    u16 *src_2;
    u32 strength;
    u32 g;
    u32 g_2;
    u32 b;
    u32 b_2;
    s32 color;
    s32 newR;
    s32 newG;
    s32 newB;
    s32 newR_2;
    s32 newG_2;
    s32 newB_2;

    buffer = &gEwramData->bgPalette;

    if (gEwramData->paletteInvert == 0)
    {
        return;
    }

    if (gEwramData->paletteInvert == 1)
    {
        gEwramData->paletteInvert = 0;
        gEwramData->bgPalette.uploaded = 0;
        return;
    }

    strength = gEwramData->paletteInvert;
    gEwramData->bgPalette.uploaded = 0;

    for (pal = 9; pal >= 0; pal--)
    {
        src = buffer->output[pal];
        dst = src;

        for (color = 0xF; color != 0; color--)
        {
            r = src[color] & 0x1F;
            g = (src[color] & (0x1F << 5)) >> 5;
            b = (src[color] & (0x1F << 10)) >> 10;

            newR = (0x1F - r) - r;
            newG = (0x1F - g) - g;
            newB = (0x1F - b) - b;

            rWeighted = newR * strength;
            if (rWeighted < 0)
            {
                rWeighted += 0xFFFF;
            }
            newR = (rWeighted >> 0x10) + r;

            gWeighted = newG * strength;
            if (gWeighted < 0)
            {
                gWeighted += 0xFFFF;
            }
            newG = (gWeighted >> 0x10) + g;

            bWeighted = newB * strength;
            if (bWeighted < 0)
            {
                bWeighted += 0xFFFF;
            }
            newB = (bWeighted >> 0x10) + b;

            dst[color] = (newR & 0x1F) | ((newG & 0x1F) << 5) | ((newB & 0x1F) << 10);
        }
    }

    buffer = &gEwramData->objPalette;
    gEwramData->objPalette.uploaded = 0;

    for (pal = 10; pal < 14; pal++)
    {
        src_2 = buffer->output[pal];
        dst_2 = src_2;

        for (color = 0xF; color != 0; color--)
        {
            r_2 = src_2[color] & 0x1F;
            g_2 = (src_2[color] & (0x1F << 5)) >> 5;
            b_2 = (src_2[color] & (0x1F << 10)) >> 10;

            newR_2 = (0x1F - r_2) - r_2;
            newG_2 = (0x1F - g_2) - g_2;
            newB_2 = (0x1F - b_2) - b_2;

            rWeighted_2 = newR_2 * strength;
            if (rWeighted_2 < 0)
            {
                rWeighted_2 += 0xFFFF;
            }
            newR_2 = (rWeighted_2 >> 0x10) + r_2;

            gWeighted_2 = newG_2 * strength;
            if (gWeighted_2 < 0)
            {
                gWeighted_2 += 0xFFFF;
            }
            newG_2 = (gWeighted_2 >> 0x10) + g_2;

            bWeighted_2 = newB_2 * strength;
            if (bWeighted_2 < 0)
            {
                bWeighted_2 += 0xFFFF;
            }
            newB_2 = (bWeighted_2 >> 0x10) + b_2;

            dst_2[color] = (newR_2 & 0x1F) | ((newG_2 & 0x1F) << 5) | ((newB_2 & 0x1F) << 0xA);
        }
    }

    gEwramData->paletteInvert = 0;
}

/**
 * @brief 3C674 | Convert BG palettes 0-9 to a tinted grayscale
 *
 * Works on the base palettes (colors 1-15). The gray level is
 * (r + g + b) * 5 / 16, then scaled per channel by 10/6, 10/7 and 3/2.
 * Used by room loading when the room flag 0x800 is set.
 */
void BgPalette_ApplyTintedGrayscale(void)
{
    struct PaletteBuffer *buffer;
    s32 pal;
    u16 *colors;
    u32 b;
    u32 g;
    u32 gray;
    u32 r;
    s32 color;
    u32 srcR;
    u32 srcG;
    u32 srcB;

    buffer = &gEwramData->bgPalette;

    for (pal = 0; pal < 10; pal++)
    {
        colors = buffer->base[pal];

        for (color = 0xF; color != 0; color--)
        {
            srcR = colors[color] & 0x1F;
            srcG = (colors[color] & (0x1F << 5)) >> 5;
            srcB = (colors[color] & (0x1F << 10)) >> 10;

            gray = ((srcR + srcG + srcB) * 5) >> 4;
            if (gray > 0x1F)
            {
                gray = 0x1F;
            }

            r = (gray * 10) / 6;
            g = (gray * 10) / 7;
            b = (gray * 3) / 2;
            if (r > 0x1F)
            {
                r = 0x1F;
            }
            if (g > 0x1F)
            {
                g = 0x1F;
            }
            if (b > 0x1F)
            {
                b = 0x1F;
            }

            colors[color] = (r & 0x1F) | ((g & 0x1F) << 5) | ((b & 0x1F) << 0xA);
        }
    }

    gEwramData->bgPalette.uploaded = 0;
}

/**
 * @brief 3C74C | Load a whole palette resource into objPalette at its default slot
 *
 * @param palette Palette resource (see PALETTE_RESOURCE_COUNT/SLOT)
 */
void ObjPalette_Load(u8 *palette)
{
    s32 count;

    if (PALETTE_RESOURCE_COUNT(palette) <= 0x10)
    {
        count = PALETTE_RESOURCE_COUNT(palette);
    }
    else
    {
        count = 0x10;
    }
    if ((PALETTE_RESOURCE_SLOT(palette) + count) > 0x10)
    {
        count = 0x10 - PALETTE_RESOURCE_SLOT(palette);
    }

    DMA_COPY_16(3, palette + 4, &gEwramData->objPalette.base[PALETTE_RESOURCE_SLOT(palette)], count * 0x20);
    gEwramData->objPalette.uploaded = 0;
    gEwramData->objPalette.dirtyMask = 0xF;
}

/**
 * @brief 3C7B4 | Copy palettes of a palette resource into objPalette
 *
 * @param palette Palette resource (byte 2 = number of palettes, data at +4)
 * @param first First palette of the resource to copy
 * @param count Number of palettes to copy (clamped to the buffer and the resource)
 * @param slot First destination slot (0-15)
 */
void ObjPalette_LoadRange(u8 *palette, u16 first, u16 count, u16 slot)
{
    s32 i;
    s32 n;

    n = count;
    if (n > 0x10)
        n = 0x10;

    if (slot + n > 0x10)
    {
        n = 0x10 - slot;
        if (n == 0)
            return;
    }
    if (first + n > palette[2])
    {
        n = palette[2] - first;
        if (n == 0)
            return;
    }

    if (n != 0)
    {
        DMA_COPY_16(3, palette + 4 + first * 0x20, &gEwramData->objPalette.base[slot], n * 0x20);
        gEwramData->objPalette.uploaded = 0;
    }

    for (i = slot; i < slot + n; i++)
        gEwramData->objPalette.dirtyMask |= 1 << i;
}

/**
 * @brief 3C858 | Set one color of an objPalette base palette
 *
 * @param slot Palette (0-15)
 * @param index Color index (0-15)
 * @param color BGR555 color
 */
void ObjPalette_SetColor(u16 slot, u16 index, u16 color)
{
    if ((slot < 0x10) && (index < 0x10))
    {
        gEwramData->objPalette.base[slot][index] = color;
        gEwramData->objPalette.uploaded = 0;
        gEwramData->objPalette.dirtyMask |= 1 << slot;
    }
}

/**
 * @brief 3C8B0 | Load a whole palette resource into bgPalette at its default slot
 *
 * @param palette Palette resource (see PALETTE_RESOURCE_COUNT/SLOT)
 */
void BgPalette_Load(u8 *palette)
{
    s32 count;

    if (PALETTE_RESOURCE_COUNT(palette) <= 0x10U)
    {
        count = PALETTE_RESOURCE_COUNT(palette);
    }
    else
    {
        count = 0x10;
    }
    if ((PALETTE_RESOURCE_SLOT(palette) + count) > 0x10)
    {
        count = 0x10 - PALETTE_RESOURCE_SLOT(palette);
    }

    DMA_COPY_16(3, palette + 4, gEwramData->bgPalette.base[PALETTE_RESOURCE_SLOT(palette)], count * 0x20);
    gEwramData->bgPalette.uploaded = 0;
    gEwramData->bgPalette.dirtyMask = 0xF;
}

/**
 * @brief 3C918 | Copy palettes of a palette resource into bgPalette
 *
 * @param palette Palette resource (byte 2 = number of palettes, data at +4)
 * @param first First palette of the resource to copy
 * @param count Number of palettes to copy (clamped to the buffer and the resource)
 * @param slot First destination slot (0-15)
 */
void BgPalette_LoadRange(u8 *palette, u16 first, u16 count, u16 slot)
{
    s32 i;
    s32 n;

    n = count;
    if (n > 0x10)
        n = 0x10;

    if (slot + n > 0x10)
    {
        n = 0x10 - slot;
        if (n == 0)
            return;
    }
    if (first + n > palette[2])
    {
        n = palette[2] - first;
        if (n == 0)
            return;
    }

    if (n != 0)
    {
        DMA_COPY_16(3, palette + 4 + first * 0x20, &gEwramData->bgPalette.base[slot], n * 0x20);
        gEwramData->bgPalette.uploaded = 0;
    }

    for (i = slot; i < slot + n; i++)
        gEwramData->bgPalette.dirtyMask |= 1 << i;
}

/**
 * @brief 3C9BC | Back up one objPalette base palette into 'saved'
 *
 * @param slot Palette (0-15)
 * @return s32 1 if saved, 0 if the slot already holds a backup
 */
s32 ObjPalette_SaveSlot(u16 slot)
{
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->objPalette;
    if (!((buffer->savedMask >> slot) & 1))
    {
        DMA_COPY_16(3, &buffer->base[slot], &buffer->saved[slot], 0x20);
        buffer->savedMask |= 1 << slot;
        return 1;
    }
    return 0;
}

/**
 * @brief 3CA20 | Back up all objPalette base palettes into 'saved'
 *
 * @return s32 1 if saved, 0 if any slot already holds a backup
 */
s32 ObjPalette_SaveAll(void)
{
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->objPalette;
    if (buffer->savedMask == 0)
    {
        DMA_COPY_16(3, &buffer->base, &buffer->saved, 0x200);
        buffer->savedMask = 0xFFFF;
        return 1;
    }
    return 0;
}

/**
 * @brief 3CA70 | Back up one bgPalette base palette into 'saved'
 *
 * @param slot Palette (0-15)
 * @return s32 1 if saved, 0 if the slot already holds a backup
 */
s32 BgPalette_SaveSlot(u16 slot)
{
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->bgPalette;
    if (!((buffer->savedMask >> slot) & 1))
    {
        DMA_COPY_16(3, &buffer->base[slot], &buffer->saved[slot], 0x20);
        buffer->savedMask |= 1 << slot;
        return 1;
    }
    return 0;
}

/**
 * @brief 3CAD4 | Back up all bgPalette base palettes into 'saved'
 *
 * @return s32 1 if saved, 0 if any slot already holds a backup
 */
s32 BgPalette_SaveAll(void)
{
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->bgPalette;
    if (buffer->savedMask == 0)
    {
        DMA_COPY_16(3, &buffer->base, &buffer->saved, 0x200);
        buffer->savedMask = 0xFFFF;
        return 1;
    }
    return 0;
}

/**
 * @brief 3CB24 | Restore one objPalette base palette from 'saved'
 *
 * @param slot Palette (0-15)
 * @return s32 1 if restored, 0 if the slot holds no backup
 */
s32 ObjPalette_RestoreSlot(u16 slot)
{
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->objPalette;
    if (!((buffer->savedMask >> slot) & 1))
        return 0;

    DMA_COPY_16(3, &buffer->saved[slot], &buffer->base[slot], 0x20);
    buffer->savedMask &= ~(1 << slot);
    buffer->uploaded = 0;
    return 1;
}

/**
 * @brief 3CB94 | Restore every backed-up objPalette palette
 *
 * @return s32 1 if anything was restored
 */
s32 ObjPalette_RestoreSaved(void)
{
    return Palette_RestoreSaved(&gEwramData->objPalette);
}

/**
 * @brief 3CBB0 | Restore one bgPalette base palette from 'saved'
 *
 * @param slot Palette (0-15)
 * @return s32 1 if restored, 0 if the slot holds no backup
 */
s32 BgPalette_RestoreSlot(u16 slot)
{
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->bgPalette;
    if (!((buffer->savedMask >> slot) & 1))
        return 0;

    DMA_COPY_16(3, &buffer->saved[slot], &buffer->base[slot], 0x20);
    buffer->savedMask &= ~(1 << slot);
    buffer->uploaded = 0;
    return 1;
}

/**
 * @brief 3CC20 | Restore every backed-up bgPalette palette
 *
 * @return s32 1 if anything was restored
 */
s32 BgPalette_RestoreSaved(void)
{
    return Palette_RestoreSaved(&gEwramData->bgPalette);
}

/**
 * @brief 3CC3C | Clear all palette animations and their backups
 */
void PaletteAnim_Init(void)
{
    DMA_FILL_16(3, 0, &gEwramData->paletteAnim, 0x184);
}

/**
 * @brief 3CC70 | Start a palette animation in an objPalette slot
 *
 * @param palette Palette resource providing the animation frames
 * @param script Animation script (see struct PaletteAnimSlot)
 * @param slot Destination palette (0-15)
 * @return s32 Always 1
 */
s32 PaletteAnim_Start(u8 *palette, u8 *script, u16 slot)
{
    gEwramData->paletteAnim.anims[slot].palette = palette;
    gEwramData->paletteAnim.anims[slot].script = script;
    gEwramData->paletteAnim.anims[slot].timer = 0;
    gEwramData->paletteAnim.anims[slot].frame = 0;
    return 1;
}

/**
 * @brief 3CCBC | Stop the palette animation of an objPalette slot
 *
 * @param slot Palette (0-15)
 */
void PaletteAnim_Stop(u16 slot)
{
    gEwramData->paletteAnim.anims[slot].palette = NULL;
    gEwramData->paletteAnim.anims[slot].script = NULL;
    gEwramData->paletteAnim.anims[slot].timer = 0;
    gEwramData->paletteAnim.anims[slot].frame = 0;
}

/**
 * @brief 3CD08 | Back up and stop all 16 palette animations, unless one is backed up
 *
 * Bits 0-15 of savedMask are per-slot flags; afterwards all of them are set.
 *
 * @return s32 1 if done, 0 if a slot flag was already set
 */
s32 PaletteAnim_SaveAll(void)
{
    struct PaletteAnimState *state = &gEwramData->paletteAnim;
    s32 mask = 0xFFFF;

    if (state->savedMask & mask)
        return 0;

    DMA_COPY_16(3, &gEwramData->paletteAnim.anims, &gEwramData->paletteAnim.savedAnims, 0xC0);
    DMA_FILL_16(3, 0, &gEwramData->paletteAnim.anims, 0xC0);
    gEwramData->paletteAnim.savedMask |= mask;
    return 1;
}

/**
 * @brief 3CD74 | Restore one backed-up palette animation
 *
 * @param slot Palette (0-15)
 * @return s32 1 if restored, 0 if the slot holds no backup
 */
s32 PaletteAnim_RestoreSlot(u16 slot)
{
    struct PaletteAnimState *state;
    s32 bit;

    state = &gEwramData->paletteAnim;
    bit = 1 << slot;
    if ((state->savedMask & bit))
    {
        DMA_COPY_16(3, &gEwramData->paletteAnim.savedAnims[slot], &gEwramData->paletteAnim.anims[slot], 0xC);
        gEwramData->paletteAnim.savedMask &= ~bit;
        return 1;
    }
    return 0;
}

/**
 * @brief 3CDDC | Per-frame palette update (called from the main loop)
 *
 * Runs the palette animations, computes the output palettes from the fade
 * state and applies the invert effect. The upload happens in VBlank
 * (Palette_TransferToPalram).
 */
void Palette_Update(void)
{
    PaletteAnim_Update();
    Palette_UpdateFades();
    Palette_ApplyInvert();
}

/**
 * @brief 3CDF0 | Start fading all objPalette palettes toward a solid color
 *
 * @param r Target color red (0-31)
 * @param g Target color green (0-31)
 * @param b Target color blue (0-31)
 * @param from Start weight of the base palettes (16.16, 0x10000 = no fade)
 * @param to End weight of the base palettes (16.16)
 * @param frames Duration in frames
 */
void ObjPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames)
{
    struct PaletteFade *fade;
    s32 step;

    fade = &gEwramData->objPalette.fade;
    step = to - from;
    step = FixedDivide(step, frames << 0x10);
    fade->activeMask |= 0x10000;
    fade->runningMask |= 0x10000;
    fade->pendingMask |= 0x10000;
    fade->colorR = r;
    fade->colorG = g;
    fade->colorB = b;
    fade->weight[PALETTE_FADE_COLOR] = from;
    fade->targetWeight[PALETTE_FADE_COLOR] = to;
    fade->step[PALETTE_FADE_COLOR] = step;
}

/**
 * @brief 3CE90 | Pause or resume the objPalette solid-color fade
 *
 * @param pause Nonzero to pause, 0 to resume
 */
void ObjPalette_PauseColorFade(s32 pause)
{
    if (pause != 0)
    {
        gEwramData->objPalette.fade.runningMask &= ~0x10000;
    }
    else
    {
        gEwramData->objPalette.fade.runningMask |= 0x10000;
    }

}

/**
 * @brief 3CED4 | Stop and reset the objPalette solid-color fade
 */
void ObjPalette_StopColorFade(void)
{
    struct PaletteBuffer *pal = &gEwramData->objPalette;
    struct PaletteFade *fade = &pal->fade;
    u32 mask = ~0x10000;

    fade->activeMask &= mask;
    fade->runningMask &= mask;
    fade->pendingMask &= mask;
    WRITE_32(&pal->fade.colorR, 0);
    pal->fade.weight[PALETTE_FADE_COLOR] = 0x10000;
    pal->fade.targetWeight[PALETTE_FADE_COLOR] = 0x10000;
    pal->fade.step[PALETTE_FADE_COLOR] = 0;
    pal->uploaded = 0;
}

/**
 * @brief 3CF34 | Start blending one objPalette palette toward a resource palette
 *
 * @param slot Palette to blend (0-15)
 * @param palette Palette resource holding the target palette
 * @param index Palette of the resource to blend toward
 * @param from Start weight of the base palette (16.16, 0x10000 = no blend)
 * @param to End weight of the base palette (16.16)
 * @param frames Duration in frames
 */
void ObjPalette_StartBlend(s32 slot, u16 *palette, s32 index, s32 from, s32 to, s32 frames)
{
    s32 step;
    struct PaletteFade *fade;

    fade = &gEwramData->objPalette.fade;
    step = to - from;
    step = FixedDivide(step, frames << 0x10);
    fade->activeMask |= 1 << slot;
    fade->runningMask |= 1 << slot;
    fade->pendingMask |= 1 << slot;
    /* Resource palette data starts after the 4-byte header (2 halfwords). */
    fade->target[slot] = palette + ((index << 4) + 2);
    fade->weight[slot] = from;
    fade->targetWeight[slot] = to;
    fade->step[slot] = step;
}

/**
 * @brief 3CFC0 | Pause or resume the blend of one objPalette palette
 *
 * @param slot Palette (0-15)
 * @param pause Nonzero to pause, 0 to resume
 */
void ObjPalette_PauseBlend(s32 slot, s32 pause)
{
    if (pause != 0)
    {
        gEwramData->objPalette.fade.runningMask &= ~(1 << slot);
    }
    else
    {
        gEwramData->objPalette.fade.runningMask |= (1 << slot);
    }
}

/**
 * @brief 3D004 | Stop and reset the blend of one objPalette palette
 *
 * @param slot Palette (0-15)
 */
void ObjPalette_StopBlend(s32 slot)
{
    struct PaletteFade *fade;
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->objPalette;
    fade = &buffer->fade;
    fade->activeMask &= ~(1 << slot);
    fade->runningMask &= ~(1 << slot);
    fade->pendingMask &= ~(1 << slot);
    fade->target[slot] = NULL;
    fade->weight[slot] = 0x10000;
    fade->targetWeight[slot] = 0x10000;
    fade->step[slot] = 0;
    buffer->uploaded = 0;
}

/**
 * @brief 3D06C | Get the objPalette fades that are still in progress
 *
 * @return s32 Bit N set: fade slot N has not reached its target (bit 16 = color fade)
 */
s32 ObjPalette_GetPendingFades(void)
{
    return gEwramData->objPalette.fade.pendingMask;
}

/**
 * @brief 3D080 | Move the objPalette fade state to savedFade and clear it
 *
 * @return s32 1 if saved, 0 if a saved state already exists
 */
s32 ObjPalette_SaveFades(void)
{
    struct PaletteFade *fade;
    struct PaletteFade *savedFade;

    fade = &gEwramData->objPalette.fade;
    savedFade = &gEwramData->objPalette.savedFade;
    if (gEwramData->objPalette.fadeSaved == 0)
    {
        gEwramData->objPalette.fadeSaved = 1;
        DMA_COPY_16(3, fade, savedFade, 0x11C);
        DMA_FILL_16(3, 0, fade, 0x11C);
        return 1;
    }
    return 0;
}

/**
 * @brief 3D0E4 | Restore the objPalette fade state from savedFade
 *
 * @return s32 1 if restored, 0 if no saved state exists
 */
s32 ObjPalette_RestoreFades(void)
{
    struct PaletteFade *fade;
    struct PaletteFade *savedFade;
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->objPalette;
    fade = &buffer->fade;
    savedFade = &buffer->savedFade;
    if (buffer->fadeSaved != 0)
    {
        DMA_COPY_16(3, savedFade, fade, 0x11C);
        DMA_FILL_16(3, 0, savedFade, 0x11C);
        buffer->fadeSaved = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief 3D148 | Pause or resume all objPalette fades (slots 0-16)
 *
 * @param pause Nonzero to pause, 0 to resume
 */
void ObjPalette_PauseAllFades(s32 pause)
{
    if (pause != 0)
    {
        gEwramData->objPalette.fade.runningMask &= ~0x1FFFF;
    }
    else
    {
        gEwramData->objPalette.fade.runningMask |= 0x1FFFF;
    }
}

/**
 * @brief 3D18C | Start fading all bgPalette palettes toward a solid color
 *
 * @param r Target color red (0-31)
 * @param g Target color green (0-31)
 * @param b Target color blue (0-31)
 * @param from Start weight of the base palettes (16.16, 0x10000 = no fade)
 * @param to End weight of the base palettes (16.16)
 * @param frames Duration in frames
 */
void BgPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames)
{
    s32 step;
    struct PaletteFade *fade;

    fade = &gEwramData->bgPalette.fade;
    step = to - from;
    step = FixedDivide(step, frames << 0x10);
    fade->activeMask |= 0x10000;
    fade->runningMask |= 0x10000;
    fade->pendingMask |= 0x10000;
    fade->colorR = r;
    fade->colorG = g;
    fade->colorB = b;
    fade->weight[PALETTE_FADE_COLOR] = from;
    fade->targetWeight[PALETTE_FADE_COLOR] = to;
    fade->step[PALETTE_FADE_COLOR] = step;
}

/**
 * @brief 3D22C | Pause or resume the bgPalette solid-color fade
 *
 * @param pause Nonzero to pause, 0 to resume
 */
void BgPalette_PauseColorFade(s32 pause)
{
    if (pause != 0)
    {
        gEwramData->bgPalette.fade.runningMask &= ~0x10000;
    }
    else
    {
        gEwramData->bgPalette.fade.runningMask |= 0x10000;
    }
}

/**
 * @brief 3D270 | Stop and reset the bgPalette solid-color fade
 */
void BgPalette_StopColorFade(void)
{
    struct PaletteBuffer *pal = &gEwramData->bgPalette;
    struct PaletteFade *fade = &pal->fade;
    u32 mask = ~0x10000;

    fade->activeMask &= mask;
    fade->runningMask &= mask;
    fade->pendingMask &= mask;
    WRITE_32(&pal->fade.colorR, 0);
    pal->fade.weight[PALETTE_FADE_COLOR] = 0x10000;
    pal->fade.targetWeight[PALETTE_FADE_COLOR] = 0x10000;
    pal->fade.step[PALETTE_FADE_COLOR] = 0;
    pal->uploaded = 0;
}

/**
 * @brief 3D2D0 | Start blending one bgPalette palette toward a resource palette
 *
 * @param slot Palette to blend (0-15)
 * @param palette Palette resource holding the target palette
 * @param index Palette of the resource to blend toward
 * @param from Start weight of the base palette (16.16, 0x10000 = no blend)
 * @param to End weight of the base palette (16.16)
 * @param frames Duration in frames
 */
void BgPalette_StartBlend(s32 slot, u16 *palette, s32 index, s32 from, s32 to, s32 frames)
{
    s32 step;
    struct PaletteFade *fade;

    fade = &gEwramData->bgPalette.fade;
    step = to - from;
    step = FixedDivide(step, frames << 0x10);
    fade->activeMask |= (1 << slot);
    fade->runningMask |= (1 << slot);
    fade->pendingMask |= (1 << slot);
    /* Resource palette data starts after the 4-byte header (2 halfwords). */
    fade->target[slot] = palette + ((index << 4) + 2);
    fade->weight[slot] = from;
    fade->targetWeight[slot] = to;
    fade->step[slot] = step;
}

/**
 * @brief 3D35C | Pause or resume the blend of one bgPalette palette
 *
 * @param slot Palette (0-15)
 * @param pause Nonzero to pause, 0 to resume
 */
void BgPalette_PauseBlend(s32 slot, s32 pause)
{
    if (pause != 0)
    {
        gEwramData->bgPalette.fade.runningMask &= ~(1 << slot);
    }
    else
    {
        gEwramData->bgPalette.fade.runningMask |= (1 << slot);
    }
}

/**
 * @brief 3D3A0 | Stop and reset the blend of one bgPalette palette
 *
 * @param slot Palette (0-15)
 */
void BgPalette_StopBlend(s32 slot)
{
    struct PaletteFade *fade;
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->bgPalette;
    fade = &buffer->fade;
    fade->activeMask &= ~(1 << slot);
    fade->runningMask &= ~(1 << slot);
    fade->pendingMask &= ~(1 << slot);
    fade->target[slot] = NULL;
    fade->weight[slot] = 0x10000;
    fade->targetWeight[slot] = 0x10000;
    fade->step[slot] = 0;
    buffer->uploaded = 0;
}

/**
 * @brief 3D408 | Get the bgPalette fades that are still in progress
 *
 * @return s32 Bit N set: fade slot N has not reached its target (bit 16 = color fade)
 */
s32 BgPalette_GetPendingFades(void)
{
    return gEwramData->bgPalette.fade.pendingMask;
}

/**
 * @brief 3D41C | Move the bgPalette fade state to savedFade and clear it
 *
 * @return s32 1 if saved, 0 if a saved state already exists
 */
s32 BgPalette_SaveFades(void)
{
    struct PaletteFade *fade;
    struct PaletteFade *savedFade;

    fade = &gEwramData->bgPalette.fade;
    savedFade = &gEwramData->bgPalette.savedFade;

    if (gEwramData->bgPalette.fadeSaved == 0)
    {
        gEwramData->bgPalette.fadeSaved = 1;
        DMA_COPY_16(3, fade, savedFade, 0x11C);
        DMA_FILL_16(3, 0, fade, 0x11C);
        return 1;
    }
    return 0;
}

/**
 * @brief 3D480 | Restore the bgPalette fade state from savedFade
 *
 * @return s32 1 if restored, 0 if no saved state exists
 */
s32 BgPalette_RestoreFades(void)
{
    struct PaletteFade *fade;
    struct PaletteFade *savedFade;
    struct PaletteBuffer *buffer;

    buffer = &gEwramData->bgPalette;
    fade = &buffer->fade;
    savedFade = &buffer->savedFade;

    if (buffer->fadeSaved != 0)
    {
        DMA_COPY_16(3, savedFade, fade, 0x11C);
        DMA_FILL_16(3, 0, savedFade, 0x11C);
        buffer->fadeSaved = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief 3D4E4 | Pause or resume all bgPalette fades (slots 0-16)
 *
 * @param pause Nonzero to pause, 0 to resume
 */
void BgPalette_PauseAllFades(s32 pause)
{
    if (pause != 0)
    {
        gEwramData->bgPalette.fade.runningMask &= ~0x1FFFF;
    }
    else
    {
        gEwramData->bgPalette.fade.runningMask |= 0x1FFFF;
    }
}

/**
 * @brief 3D528 | Make the next PaletteAnim_Update do nothing
 */
void PaletteAnim_SkipFrame(void)
{
    gEwramData->paletteAnimSkipFrame = 1;
}

/**
 * @brief 3D544 | Back up the invert strength and turn the effect off
 */
void Palette_SuspendInvert(void)
{
    gEwramData->paletteInvertSaved = gEwramData->paletteInvert;
    gEwramData->paletteInvert = 0;
}

/**
 * @brief 3D564 | Restore the invert strength backed up by Palette_SuspendInvert
 */
void Palette_ResumeInvert(void)
{
    gEwramData->paletteInvert = gEwramData->paletteInvertSaved;
}

/*
 * OBJ tile slot allocator.
 *
 * OBJ VRAM is divided into slots (gEwramData->objTileSlots) of 4 tile rows
 * each; graphics are loaded into a run of consecutive slots. 'size' is the
 * number of slots one graphic needs (1, 2 or 4) and 'count' how many such
 * graphics (frames) are reserved together; a run of size * count slots is
 * marked used and every slot of it remembers size and count.
 *
 * gEwramData->objTileFlags selects the layout (OBJ_TILES_*):
 *   GRID     - slots use the 2D grid layout of sObjTileGrid4bpp/8bpp (set by
 *              ObjTiles_Init); otherwise slots are linear runs of tiles,
 *   COLOR256 - 8bpp graphics: slots are twice as large, half as many (8),
 *   BITMAP   - bitmap BG mode: only the upper half of OBJ VRAM (tiles from
 *              0x200) is usable, so only half the slots exist.
 */

/**
 * @brief Convert a start and span length to the inclusive last slot.
 *
 * The two-step end calculation and inline boundary keep agbcc from
 * reducing the caller's last-slot calculation as an induction variable.
 * This preserves matching output without a dead assignment.
 */
static inline s32 LastAllocationSlot(s32 start, s32 length)
{
    s32 end = start + length;
    end--;
    return end;
}

/**
 * @brief 3D580 | Reserve the first free run of slots, searching from slot 0
 *
 * Runs start at multiples of size. The bound check allows a last slot of 16
 * (one past the table), as in the original code.
 *
 * @param size Slots per graphic; also the search alignment.
 * @param count Number of graphics to reserve.
 * @return First allocated slot, or -1 when no run is free.
 */
s32 ObjTiles_AllocForward(s32 size, s32 count)
{
    s32 last;
    s32 available;
    s32 slot;
    s32 start;
    s32 total;

    total = (size * count);
    for (start = 0; start < 0x10; start += size)
    {
        last = LastAllocationSlot(start, total);
        if (last > 0x10)
            return -1;

        available = 1;
        for (slot = last; slot >= start; slot--)
        {
            if (gEwramData->objTileSlots[slot].used != 0)
            {
                available = 0;
                break;
            }
        }

        if (available != 0)
        {
            for (slot = start; slot <= last; slot++)
            {
                gEwramData->objTileSlots[slot].used = 1;
                gEwramData->objTileSlots[slot].size = size;
                gEwramData->objTileSlots[slot].count = count;
            }
            return start;
        }
    }

    return -1;
}

/**
 * @brief 3D640 | Reserve the last free run of slots, searching from the end
 *
 * @param size Slots per graphic; the run end moves back by size per try.
 * @param count Number of graphics to reserve.
 * @return First allocated slot, or -1 when no run is free.
 */
s32 ObjTiles_AllocBackward(s32 size, s32 count)
{
    s32 start;
    s32 available;
    s32 slot;
    s32 last;
    s32 total;

    total = (size * count);
    last = gEwramData->objTileSlotCount - 1;
    do
    {
        start = (last - total) + 1;
        if (start < 0)
            return -1;

        available = 1;
        for (slot = start; slot <= last; slot++)
        {
            if (gEwramData->objTileSlots[slot].used != 0)
            {
                available = 0;
                break;
            }
        }

        if (available != 0)
        {
            for (slot = start; slot <= last; slot++)
            {
                gEwramData->objTileSlots[slot].used = 1;
                gEwramData->objTileSlots[slot].size = size;
                gEwramData->objTileSlots[slot].count = count;
            }
            return start;
        }
        last -= size;
    }
    while (last >= 0);

    return -1;
}

/**
 * @brief 3D714 | Free all slots and compute their tiles for the current flags
 *
 * 16 slots (8 with OBJ_TILES_COLOR256), halved in OBJ_TILES_BITMAP mode; the
 * slots past objTileSlotCount are marked used so they are never allocated.
 */
void ObjTiles_ResetSlots(void)
{
    s16 slot;
    s16 slotCount;

    slotCount = 4;
    if (!((gEwramData->objTileFlags >> 1) & 1))
    {
        slotCount = 8;
    }
    if (ObjTiles_IsBitmapMode() == 0)
    {
        slotCount *= 2;
    }
    gEwramData->objTileSlotCount = slotCount;

    for (slot = 0; slot < slotCount; slot++)
    {
        gEwramData->objTileSlots[slot].used = 0;
        gEwramData->objTileSlots[slot].size = 0;
        gEwramData->objTileSlots[slot].count = 0;
        gEwramData->objTileSlots[slot].firstTile = ObjTiles_GetSlotTile(slot);
    }

    for (slot = slotCount; slot < 0x10; slot++)
    {
        gEwramData->objTileSlots[slot].used = 1;
        gEwramData->objTileSlots[slot].size = 0;
        gEwramData->objTileSlots[slot].count = 0;
    }
}

/**
 * @brief 3D808 | Reserve a run of slots at a given position
 *
 * @param start First slot of the run
 * @param size Slots per graphic
 * @param count Number of graphics
 * @return s32 start, or -1 if a slot of the run is used
 */
s32 ObjTiles_AllocAt(s32 start, s32 size, s32 count)
{
    s32 last;
    s32 slot;

    last = (start + (count * size)) - 1;
    if (last > 0x10)
    {
        return -1;
    }

    for (slot = last; slot >= start; slot--)
    {
        if (gEwramData->objTileSlots[slot].used != 0)
        {
            return -1;
        }
    }

    for (slot = start; slot <= last; slot++)
    {
        gEwramData->objTileSlots[slot].used = 1;
        gEwramData->objTileSlots[slot].size = size;
        gEwramData->objTileSlots[slot].count = count;
    }

    return start;
}

/* First tile of each slot in OBJ_TILES_GRID mode (2D mapping, 32 tiles per
 * row): 4bpp has two columns of 16 tiles x 8 bands of 4 rows, 8bpp one
 * column of 8 bands. */
extern u16 sObjTileGrid4bpp[];
extern u16 sObjTileGrid8bpp[];

/**
 * @brief 3D8B4 | Map an allocation slot to its first OBJ tile number
 *
 * @param slot Allocation-slot index.
 * @return OBJ tile number (in 32-byte units).
 */
s32 ObjTiles_GetSlotTile(s32 slot)
{
    s32 tileNumber;
    s32 tileStride;
    s32 lowBitMask;

    /* OBJ_TILES_BITMAP contributes a tile-number offset of 0x200. Retain the
     * sign-propagation expression for the original instruction sequence. */
    tileNumber = (-(OBJ_TILES_BITMAP & gEwramData->objTileFlags) >> 0x1F) & 0x200;
    tileStride = 0x40;
    /* This mask is used by both shifted-bit comparisons below. Keeping
     * it in a local prevents agbcc from folding == 1 into a zero test.
     * This does not establish the original source spelling. */
    lowBitMask = 1;
    if ((gEwramData->objTileFlags & OBJ_TILES_GRID) == 0)
    {
        if (((gEwramData->objTileFlags >> 1) & lowBitMask) == 1)
        {
            tileStride = 0x80;
        }
        tileNumber = tileNumber + (tileStride * slot);
        return tileNumber;
    }
    if (((gEwramData->objTileFlags >> 1) & lowBitMask) == 1)
    {
        tileStride = sObjTileGrid8bpp[slot];
    }
    else
    {
        tileStride = sObjTileGrid4bpp[slot];
    }
    tileNumber = tileNumber + tileStride;
    return tileNumber;
}

/**
 * @brief 3D924 | Free the whole run that starts at a slot
 *
 * @param slot First slot of the run (its size * count slots are freed)
 * @return s32 1, or 0 for an invalid slot
 */
s32 ObjTiles_Free(s32 slot)
{
    s16 i;
    s16 total;

    if (slot > 0xFU)
    {
        return 0;
    }

    total = gEwramData->objTileSlots[slot].size * gEwramData->objTileSlots[slot].count;
    for (i = slot; i < (slot + total); i++)
    {
        gEwramData->objTileSlots[i].used = 0;
        gEwramData->objTileSlots[i].size = 0;
        gEwramData->objTileSlots[i].count = 0;
    }
    return 1;
}

/**
 * @brief 3D9A8 | Set the default layout (grid, 4bpp, full OBJ VRAM) and free all slots
 */
void ObjTiles_Init(void)
{
    ObjTiles_SetGridLayout(1);
    ObjTiles_SetColor256(0);
    ObjTiles_SetBitmapMode(0);
    ObjTiles_ResetSlots();
}

/**
 * @brief 3D9C4 | Set or clear OBJ_TILES_GRID
 *
 * @param enable 1 = grid slot layout, 0 = linear
 */
void ObjTiles_SetGridLayout(u16 enable)
{
    gEwramData->objTileFlags = (gEwramData->objTileFlags & ~OBJ_TILES_GRID) | (enable & 1);
}

/**
 * @brief 3D9EC | Set or clear OBJ_TILES_COLOR256
 *
 * @param enable 1 = 8bpp graphics
 */
void ObjTiles_SetColor256(u16 enable)
{
    gEwramData->objTileFlags = (gEwramData->objTileFlags & ~OBJ_TILES_COLOR256) | ((enable << 1) & OBJ_TILES_COLOR256);
}

/**
 * @brief 3DA14 | Set or clear OBJ_TILES_BITMAP
 *
 * @param enable 1 = only the upper half of OBJ VRAM is usable
 */
void ObjTiles_SetBitmapMode(u16 enable)
{
    gEwramData->objTileFlags = (gEwramData->objTileFlags & ~OBJ_TILES_BITMAP) | ((enable << 2) & OBJ_TILES_BITMAP);
}

/**
 * @brief 3DA3C | Is OBJ_TILES_BITMAP set?
 *
 * @return s32 1 or 0
 */
s32 ObjTiles_IsBitmapMode(void)
{
    return (gEwramData->objTileFlags >> 2) & 1;
}

/**
 * @brief 3DA58 | Reserve slots; runs of 4 from the start, smaller ones from the end
 *
 * @param size Slots per graphic (1, 2 or 4)
 * @param count Number of graphics
 * @param fallback Returned for any other size
 * @return s32 First allocated slot, -1 if none is free
 */
s32 ObjTiles_AllocPreferEnd(s32 size, s32 count, s32 fallback)
{
    switch (size)
    {
        case 4:
            fallback = ObjTiles_AllocForward(size, count);
            break;

        case 1:
        case 2:
            fallback = ObjTiles_AllocBackward(size, count);
            break;
    }

    return fallback;
}

/**
 * @brief 3DA7C | Get the OBJ tile number of a pixel position inside a slot
 *
 * Assumes the 2D mapping (32 tile units per row); in 8bpp a tile is two
 * units wide.
 *
 * @param slot Allocation slot
 * @param x X offset in pixels
 * @param y Y offset in pixels
 * @return s32 OBJ tile number
 */
s32 ObjTiles_GetTile(s32 slot, u32 x, u32 y)
{
    s32 firstTile;
    s32 color256;
    s32 column;
    s32 row;

    firstTile = gEwramData->objTileSlots[slot].firstTile;
    color256 = (gEwramData->objTileFlags >> 1) & 1;
    column = (x >> 3) << color256;
    row = (y >> 3) << 5;
    return column + row + firstTile;
}

/**
 * @brief 3DAB4 | Reserve slots for graphics
 *
 * @param size Slots per graphic (1, 2 or 4; runs of 4 always come from the start)
 * @param count Number of graphics
 * @param fromEnd Nonzero: runs of 1 or 2 slots are taken from the end
 * @return s32 First allocated slot, -1 if none is free
 */
s32 ObjTiles_Alloc(s32 size, s32 count, s32 fromEnd)
{
    s32 slot;

    switch (size)
    {
        case 4:
            slot = ObjTiles_AllocForward(size, count);
            break;

        case 1:
        case 2:
            if (fromEnd == 0)
            {
                slot = ObjTiles_AllocForward(size, count);
            }
            else
            {
                slot = ObjTiles_AllocBackward(size, count);
            }
            break;
    }

    // BUG: slot is uninitialized for any other size
    return slot;
}

/**
 * @brief 3DAE0 | Free size * count slots starting at a slot
 *
 * @param start First slot
 * @param size Slots per graphic
 * @param count Number of graphics
 * @return s32 1, or 0 for an invalid slot
 */
s32 ObjTiles_FreeRange(s32 start, s32 size, s32 count)
{
    s32 end;
    s32 slotCount;
    s32 slot;

    slotCount = (u16) gEwramData->objTileSlotCount;
    end = start + size * count;
    if (start > 0xFU)
    {
        return 0;
    }

    if (end > slotCount)
    {
        end = slotCount;
    }

    for (slot = start; slot < end; slot++)
    {
        gEwramData->objTileSlots[slot].used = 0;
        gEwramData->objTileSlots[slot].size = 0;
        gEwramData->objTileSlots[slot].count = 0;
    }

    return 1;
}
