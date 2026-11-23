#ifndef SOUL_MENU_H
#define SOUL_MENU_H

/* Functions of menu/soul_menu.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void MenuPageTitle_Update(struct EwramData_EntityData *entity);
extern void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item);
extern void MenuSlotIcon_Show(s32 x, s32 y, s32 type, s32 item, s32 slot);
extern void Shop_DrawCursors(struct EwramData_EntityData *entity);
extern void Cursor_StartAnimation(void);
extern void Cursor_SetTilePosition(s32 column, s32 row);
extern void MenuPageTitle_Delete(struct EwramData_EntityData *title);
extern void MenuItemIcon_Draw(void);
extern void MenuItemIcon_Delete(void);
extern void MenuSlotIcons_Draw(void);
extern void MenuSlotIcons_Delete(void);
extern void MenuSlotIcon_SetHidden(u8 value, s32 index);
extern s32 CursorSprite_Create(struct EwramData_EntityData *entity, u16 palette);
extern void SoulList_Insert(s32 kind, s32 soul);

#endif /* SOUL_MENU_H */
