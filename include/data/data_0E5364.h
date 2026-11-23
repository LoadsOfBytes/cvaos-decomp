#ifndef DATA_0E5364_H
#define DATA_0E5364_H

/* Data of asm/data/data_0E5364.s used by C files. */

#include "types.h"

extern u32 *sMainMenuDeleteBg2Map;
extern u32 *sMainMenuCopyBg2Map;
extern u32 *sMainMenuDeleteBg1Map;
extern u32 *sMainMenuCopyBg1Map;
extern u16 sMapCellRoomInfo[]; // map info
extern u8 sMapCellEdges[];
extern s8 sGameFlagMapCoordinates[][2];
extern u8 sTextBgPalette[];
extern const s32 sHudMapTiles[3];
extern const s32 sHudMapPalettes[3];
extern u8 sObject0CSolidCallbacks[];
extern u8 sObject12Variant0AnimScript0[];
extern u8 sObject12Variant0AnimScript1[];
extern u8 sObject12Variant0AnimScript2[];
extern u8 sObject14Variant1SolidCallbacks[];
extern const s16 sUnk_08118590[];
extern const u16 sObject17Xs[];
extern const u16 sObject17Ys[];
extern u16 sObject1BColors[];
/* Start x / y of object 0x2F per variant (spawnParam0). */
extern const u16 sObject2FXs[3];
extern const u16 sObject2FYs[3];
extern const u16 sObject31Xs[4];
extern const u16 sObject31Ys[4];
extern const u16 sObject31Anims[4];
extern u8 sObject36SolidCallbacks[];
/* Object 0x36 per variant: { rotation speed * 2 / scale step, rotation speed }. */
extern const s16 sObject36Shapes[9][2];
extern u8 sEventScene19Gfx[];

#endif /* DATA_0E5364_H */
