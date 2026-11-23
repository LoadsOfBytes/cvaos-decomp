#ifndef PALETTE_H
#define PALETTE_H

/* Functions of engine/palette.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Palette_ApplyInvert(void);
extern void BgPalette_ApplyTintedGrayscale(void);
extern void ObjPalette_Load(u8 *palette);
extern void ObjPalette_LoadRange(u8 *palette, u16 first, u16 count, u16 slot);
extern void ObjPalette_SetColor(u16 slot, u16 index, u16 color);
extern void BgPalette_Load(u8 *palette);
extern void BgPalette_LoadRange(u8 *palette, u16 first, u16 count, u16 slot);
extern s32 ObjPalette_SaveSlot(u16 slot);
extern s32 ObjPalette_SaveAll(void);
extern s32 BgPalette_SaveSlot(u16 slot);
extern s32 BgPalette_SaveAll(void);
extern s32 ObjPalette_RestoreSlot(u16 slot);
extern s32 ObjPalette_RestoreSaved(void);
extern s32 BgPalette_RestoreSlot(u16 slot);
extern s32 BgPalette_RestoreSaved(void);
extern void PaletteAnim_Init(void);
extern s32 PaletteAnim_Start(u8 *palette, u8 *script, u16 slot);
extern void PaletteAnim_Stop(u16 slot);
extern s32 PaletteAnim_SaveAll(void);
extern s32 PaletteAnim_RestoreSlot(u16 slot);
extern void Palette_Update(void);
extern void ObjPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames);
extern void ObjPalette_PauseColorFade(s32 pause);
extern void ObjPalette_StopColorFade(void);
extern void ObjPalette_StartBlend(s32 slot, u16 *palette, s32 index, s32 from, s32 to, s32 frames);
extern void ObjPalette_PauseBlend(s32 slot, s32 pause);
extern void ObjPalette_StopBlend(s32 slot);
extern s32 ObjPalette_GetPendingFades(void);
extern s32 ObjPalette_SaveFades(void);
extern s32 ObjPalette_RestoreFades(void);
extern void ObjPalette_PauseAllFades(s32 pause);
extern void BgPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames);
extern void BgPalette_PauseColorFade(s32 pause);
extern void BgPalette_StopColorFade(void);
extern void BgPalette_StartBlend(s32 slot, u16 *palette, s32 index, s32 from, s32 to, s32 frames);
extern void BgPalette_PauseBlend(s32 slot, s32 pause);
extern void BgPalette_StopBlend(s32 slot);
extern s32 BgPalette_GetPendingFades(void);
extern s32 BgPalette_SaveFades(void);
extern s32 BgPalette_RestoreFades(void);
extern void BgPalette_PauseAllFades(s32 pause);
extern void PaletteAnim_SkipFrame(void);
extern void Palette_SuspendInvert(void);
extern void Palette_ResumeInvert(void);
extern s32 ObjTiles_AllocForward(s32 size, s32 count);
extern s32 ObjTiles_AllocBackward(s32 size, s32 count);
extern void ObjTiles_ResetSlots(void);
extern s32 ObjTiles_AllocAt(s32 start, s32 size, s32 count);
extern s32 ObjTiles_GetSlotTile(s32 slot);
extern s32 ObjTiles_Free(s32 slot);
extern void ObjTiles_Init(void);
extern void ObjTiles_SetGridLayout(u16 enable);
extern void ObjTiles_SetColor256(u16 enable);
extern void ObjTiles_SetBitmapMode(u16 enable);
extern s32 ObjTiles_IsBitmapMode(void);
extern s32 ObjTiles_AllocPreferEnd(s32 size, s32 count, s32 fallback);
extern s32 ObjTiles_GetTile(s32 slot, u32 x, u32 y);
extern s32 ObjTiles_Alloc(s32 size, s32 count, s32 fromEnd);
extern s32 ObjTiles_FreeRange(s32 start, s32 size, s32 count);

#endif /* PALETTE_H */
