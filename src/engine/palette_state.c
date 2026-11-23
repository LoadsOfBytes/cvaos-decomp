/* Palette_SaveState and Palette_RestoreState from upstream
 * engine/obj_gfx.c; this unit is built with -fno-cse-skip-blocks (
 * both match only with it, its neighbours in engine/obj_gfx.c and
 * engine/palette.c only without it). */
#include "palette_state.h"
#include "structs/render_slots.h"
#include "gba.h"

#include "graphics_inline.h"
#include "obj_gfx.h"
#include "palette.h"

// Matching (upstream scratch: https://decomp.me/scratch/FqA9S)
/* Same as ObjPalette_SaveFades: move a fade state to its backup and clear it. */
static inline void SaveFadeState(void *fade, void *savedFade, u8 *fadeSaved)
{
    if (*fadeSaved == 0)
    {
        *fadeSaved = 1;
        DMA_COPY_16(3, fade, savedFade, 0x11C);
        DMA_FILL_16(3, 0, fade, 0x11C);
    }
}

/* Same as ObjPalette_SaveAll: copy all 16 base palettes (0x200 bytes) to
 * 'saved', which directly follows them, unless a backup exists. */
static inline void SaveAllBasePalettes(void *base, u16 *savedMask)
{
    
    if (*savedMask == 0)
    {
        DMA_COPY_16(3, base, base + 0x200, 0x200);
        { u16 v_ = 0xFFFF; *savedMask = v_; } /* value through a u16 local: loaded straight into r0 */
    }
}

/* Same as PaletteAnim_SaveAll. */
static inline void SaveAllPaletteAnims(struct EwramData *e)
{
    s32 *p = &e->paletteAnim.savedMask;
    if (!(*p & 0xFFFF))
    {
        DMA_COPY_16(3, &e->paletteAnim.anims, &e->paletteAnim.savedAnims, 0xC0);
        DMA_FILL_16(3, 0, &e->paletteAnim.anims, 0xC0);
        *p |= 0xFFFF;
    }
}

/**
 * @brief 3C294 | Back up the complete palette state
 *
 * Saves the fade states and base palettes of both buffers and all palette
 * animations, and suspends the invert effect. Used around menus and
 * cutscenes; undone by Palette_RestoreState.
 */
void Palette_SaveState(void)
{
    SaveFadeState(&gEwramData->objPalette.fade, &gEwramData->objPalette.savedFade, &gEwramData->objPalette.fadeSaved);
    SaveFadeState(&gEwramData->bgPalette.fade, &gEwramData->bgPalette.savedFade, &gEwramData->bgPalette.fadeSaved);
    SaveAllBasePalettes(&gEwramData->objPalette.base, &gEwramData->objPalette.savedMask);
    SaveAllBasePalettes(&gEwramData->bgPalette.base, &gEwramData->bgPalette.savedMask);
    SaveAllPaletteAnims(gEwramData);

    Palette_SuspendInvert();
}

// Matching with -fno-cse-skip-blocks for this file (upstream scratch: https://decomp.me/scratch/RBXEz)
/* Same as ObjPalette_RestoreFades: move the backup back and clear it. */
static inline void RestoreFadeState(void *fade, void *savedFade, u8 *fadeSaved)
{
    if (*fadeSaved != 0)
    {
        DMA_COPY_16(3, savedFade, fade, 0x11C);
        DMA_FILL_16(3, 0, savedFade, 0x11C);
        *fadeSaved = 0;
    }
}

/**
 * @brief 3C3E0 | Restore the palette state backed up by Palette_SaveState
 */
void Palette_RestoreState(void)
{
    RestoreFadeState(&gEwramData->objPalette.fade, &gEwramData->objPalette.savedFade, &gEwramData->objPalette.fadeSaved);
    RestoreFadeState(&gEwramData->bgPalette.fade, &gEwramData->bgPalette.savedFade, &gEwramData->bgPalette.fadeSaved);

    Palette_RestoreSaved(&gEwramData->objPalette);
    Palette_RestoreSaved(&gEwramData->bgPalette);

    PaletteAnim_RestoreSaved();
    Palette_ResumeInvert();
}
