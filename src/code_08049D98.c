/* Soul menu helpers 0x08049D98..0x08049E63. */
#include "code/code_08039340.h"
#include "menu_page.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern void Menu_PrintSoulName(s32 kind, s32 soul);
extern void SoulMenu_DrawStatPreview(struct EwramData_EntityData *menu);

/**
 * @brief Clear four background-map columns at 0x18 across rows two through seven.
 */
void SoulMenu_ClearStatPreview(void)
{
    s32 i;

    for (i = 0; i <= 5; i++)
        HudMap_ClearTiles(0x18, i + 2, 4);
}

/**
 * @brief Print the selected red/blue/yellow equipped-soul name or its none text in the configured two-row Text12 region.
 */
void SoulMenu_PrintEquippedName(struct EwramData_EntityData *entity)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    s32 text = 0x34F;

    switch ((s8)MENU_PAGE_CURSOR(entity))
    {
        case 0:
            if (player->equippedRedSoul != 0)
                text = player->equippedRedSoul + 0x1E3;
            else
                text = 0x37D;
            break;
        case 1:
            if (player->equippedBlueSoul != 0)
                text = player->equippedBlueSoul + 0x21B;
            else
                text = 0x37E;
            break;
        case 2:
            if (player->equippedYellowSoul != 0)
                text = player->equippedYellowSoul + 0x233;
            else
                text = 0x37F;
            break;
    }

    Text12_SetRegion(0, 0, 0x1D, 2);
    Text12_ClearRegion();
    MenuText_Print(text);
}

/**
 * @brief 49E4C | Menu_PrintSoulName(unk_4F7, unk_4FB), then SoulMenu_DrawStatPreview.
 */
void SoulMenu_ShowSelection(struct EwramData_EntityData *entity)
{
    Menu_PrintSoulName(MENU_PAGE_CATEGORY(entity), MENU_PAGE_ITEM(entity));
    SoulMenu_DrawStatPreview(entity);
}

/**
 * @brief Insert soulIndex+1 once into the kind-selected four-entry list at its first empty slot or slot three; preserve the incomplete kind guard and dispatch.
 */
void SoulList_Insert(s32 kind, s32 soul)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list;
    s32 i;

    if (kind > 7)
        return;

    soul++;
    switch (kind)
    {
        case 0:
            list = player->redSoulList;
            break;
        case 1:
            list = player->blueSoulList;
            break;
        case 2:
            list = player->yellowSoulList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == soul)
            return;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == 0xFF)
        {
            list[i] = soul;
            break;
        }
    }
    if (i == 4)
        list[3] = soul;
}

/**
 * @brief Write the value to slot zero, returning if its low byte is nonzero; otherwise write all four entries and retain the final slot-three assignment.
 */
void SoulList_WriteValue(s32 kind, s32 value)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list;
    s32 i;

    switch (kind)
    {
        case 0:
            list = player->redSoulList;
            break;
        case 1:
            list = player->blueSoulList;
            break;
        case 2:
            list = player->yellowSoulList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        list[i] = value;
        if ((u8)value != 0)
            return;
    }
    list[3] = value;
}
