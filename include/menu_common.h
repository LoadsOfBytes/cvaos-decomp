#ifndef MENU_COMMON_H
#define MENU_COMMON_H

/* Functions of menu/menu_common.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"


extern void MenuList_DrawScrollArrows(struct EwramData_EntityData *page, s32 raised);
extern void Menu_DrawHpMpBars(s32 x, s32 y);
extern void Menu_DrawCurrentStats(s32 x, s32 y);
extern void Menu_DrawAttackDefense(s32 x, s32 y);
extern void Player_ClearEquipmentAndSoulLists(void);
extern void MenuText_PrintWithInsertedEntry(s32 id, s32 arg);
extern void MenuText_Print(s32 index);
extern s32 HudMap_PrintNumberAttr(s32 x, s32 y, s32 value, s32 digits, s32 attr, s32 zeroPad);
extern s32 HudMap_PrintPaddedNumber(s32 x, s32 y, s32 value_, s32 digits);
extern s32 HudMap_PrintPaddedNumberInPalette(s32 x, s32 y, s32 value_, s32 digits, s32 palette);
extern void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count);
extern void HudMap_PrintLargeNumber(s32 x, s32 y, s32 value, s32 digits, s32 palette);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern void HudMap_ClearAll(void);
extern void HudMap_DrawStatChangeIndicator(s32 x, s32 y, s32 index);
extern void HudMap_DrawStatusConditionIcon(s32 x, s32 y);

extern void HudMap_ZeroTiles(s32 x, s32 y, s32 count);
extern void MenuPage_SetMode(struct EwramData_EntityData *entity, u8 mode);
extern s32 HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 palette, s32 zeroPad);
extern void Menu_DrawPlayTime(s32 x, s32 y, s32 palette, s32 frames);

#endif /* MENU_COMMON_H */
