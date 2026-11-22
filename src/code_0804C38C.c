/* Menu helpers 0x0804C38C..0x0804C403. */
#include "structs/ewram.h"

extern void Cursor_Draw(void);
extern void MenuPageTitle_Update(struct EwramData_EntityData *entity);
extern void ButtonConfigMenu_Draw(struct EwramData_EntityData *page);
extern void Cursor_SetTilePosition(s32 column, s32 row);

/* Default button assignment: indices into sButtonConfigButtons. */
extern const u16 sButtonConfigButtons[];
extern const u8 sButtonConfigDefaultChoices[4];

/**
 * @brief 4C38C | Cursor_Draw, then MenuPageTitle_Update on entity slot 20.
 */
void ButtonConfigMenu_DrawSprites(void)
{
    Cursor_Draw();
    MenuPageTitle_Update(&gEwramData->entityData[20]);
}

/**
 * @brief 4C3AC | ButtonConfigMenu_Draw and move the cursor (slot 18) to row unk_4F9.
 */
void ButtonConfigMenu_Redraw(struct EwramData_EntityData *entity)
{
    ButtonConfigMenu_Draw(entity);
    Cursor_SetTilePosition(5, (s8)entity->unk_4F8.part8.unk_4F9 * 2 + 4);
}

/**
 * @brief 4C3C8 | Reset the attack / jump / ability / guardian buttons.
 */
void ButtonConfig_ResetDefaults(void)
{
    u16 *buttons = &gEwramData->playerStatus.attackButton;
    s32 i;

    for (i = 0; i <= 3; i++)
        buttons[i] = sButtonConfigButtons[sButtonConfigDefaultChoices[i]];
}
