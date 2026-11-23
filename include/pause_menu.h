#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

/* Functions of menu/pause_menu.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

struct SoulTypeIcons;

extern void MenuText_ClearAndPrint(s32 index);
extern void Text12_SetupTileRegion(s32 x, s32 y, u16 tile, s32 width, s32 height);
extern void Menu_PrintSoulName(s32 soulType, s32 soulId);
extern void Text12_PrintTwoLineEntryAt(s32 index, s32 row);
extern void PauseMenu_Enter(struct GameState *gameState);
extern s32 PauseMenu_Update(struct GameState *gameState);
extern void ItemMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
extern void ButtonConfigMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
extern void BestiaryMenu_DrawCursor(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
extern void MenuCursor_LoadGraphics(struct EwramData_EntityData *entity);
extern void Shop_DrawListCursor(struct EwramData_EntityData *entity);
extern void Cursor_Draw(void);
extern void SoulTypeSelector_SetDirection(struct EwramData_EntityData *entity, s8 dir);

extern void MenuPage_Reset(struct EwramData_EntityData *entity);
extern void SoulTypeSelector_Update(struct SoulTypeIcons *e);

#endif /* PAUSE_MENU_H */
