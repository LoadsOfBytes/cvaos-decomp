#ifndef EQUIPMENT_MENU_H
#define EQUIPMENT_MENU_H

/* Functions of menu/equipment_menu.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct EquipmentMenuItemsView;
struct EquipmentSlotCursorView;
struct MenuPage2;

struct MenuPage;

extern void EquipmentMenu_PrintEquippedNames(void);
extern void EquipmentMenu_DrawStatPreview(struct MenuPage *page);
extern void Equipment_PrepareBonuses(s32 category, s32 item);
extern void Player_RefreshEquipmentStats(void);
extern void EquipmentList_Insert(s32 type, s32 item);

extern void EquipmentMenu_ShowSlotDescription(struct EquipmentMenuItemsView *stats, struct EquipmentSlotCursorView *page);
extern void EquipmentMenu_DrawEquippedIcons(struct EquipmentMenuItemsView *stats, void *unusedPage);
extern void EquipmentMenu_DrawList(struct PlayerStatus *stats, struct MenuPage2 *page, s32 slot);
extern void EquipmentMenu_CountList(struct PlayerStatus *stats, struct MenuPage2 *page, s32 slot);

#endif /* EQUIPMENT_MENU_H */
