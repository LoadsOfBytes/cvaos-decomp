/* ItemMenu_CountList, ItemMenu_Open. */
#include "code/code_08039340.h"
#include "menu_page.h"
#include "code/code_08040A38.h"
#include "menu_entity.h"
#include "structs/ewram.h"

/* Usable item records (16 bytes; see Item_GetInfo). */
struct UsableItemInfo {
    u16 name;
    u8 oneBasedIconId;
    u8 pad_3[0xD];
};

extern const struct UsableItemInfo sConsumableInfo[];
extern const u16 sItemDescriptionTextIds[];
/* Called with an int tile here (no u16 truncation in the original). */
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
extern void Menu_DrawHpMpBars(s32 x, s32 y);
extern void HudMap_DrawStatusConditionIcon(s32 x, s32 y);
extern void MenuPage_SetMode(struct EwramData_EntityData *entity, u8 mode);
extern void ItemMenu_DrawList(struct PlayerStatus *menu, struct EwramData_EntityData *entity);
extern void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item);
extern void MenuItemIcon_SetHidden(u8 value);
extern void MenuPageTitle_Create(struct EwramData_EntityData *title, s32 frame);
extern void Cursor_SetTilePosition(s32 column, s32 row);
extern void Cursor_Draw(void);

/**
 * @brief 4B648 | Count the owned item kinds (itemInventory, 32 entries)
 * into unk_4F8 of the item menu entity and store the item under the cursor
 * row unk_4F9 in unk_4FB, at most eight rows from the list top unk_4FA
 * (same scheme as SoulMenu_CountList; plain while loops keep the entry tests).
 */
void ItemMenu_CountList(struct PlayerStatus *menu, struct EwramData_EntityData *entity)
{
    s32 shown = 0;
    s32 pos = 0;
    s32 i;

    MENU_PAGE_ITEM(entity) = 0xFF;
    i = 0;
    while (pos < entity->unk_4F8.bytes[2] && i <= 0x1F)
    {
        if (menu->itemInventory[i] != 0)
            pos++;
        i++;
    }
    while (shown <= 7 && i <= 0x1F)
    {
        if (menu->itemInventory[i] != 0)
        {
            if (pos == entity->unk_4F8.bytes[1])
                MENU_PAGE_ITEM(entity) = i;
            pos++;
            shown++;
        }
        i++;
    }
    while (i <= 0x1F)
    {
        if (menu->itemInventory[i] != 0)
            pos++;
        i++;
    }
    MENU_PAGE_COUNT(entity) = pos;
}

/**
 * @brief 4B6C4 | Set up the item menu screen for entity: reset the cursor
 * state, draw the frames and the two columns of four 10-tile rows, list
 * the items (ItemMenu_DrawList), show the icon and name of the item under the
 * cursor (or the empty indicator) and the cursor itself.
 */
void ItemMenu_Open(struct EwramData_EntityData *entity)
{
    struct PlayerStatus *menu = &gEwramData->playerStatus;
    s32 i;
    s32 tile;
    s32 right;
    s32 left;
    s32 y;

    MENU_PAGE_TOP(entity) = 0;
    MENU_PAGE_CURSOR(entity) = 0;
    MENU_PAGE_ITEM(entity) = 0xFF;
    MENU_SAVED_TOP(entity) = 0;
    Text12_ClearRegion();
    for (i = 0, tile = 0x3200; i <= 2; i++)
    {
        HudMap_WriteTiles(5, i + 0x10, tile, 0x18);
        tile += 0x20;
    }
    Text12_SetRegion(0, 0, 0x20, 2);
    for (i = 0, right = 0x3390, left = 0x3380; i <= 3; i++)
    {
        y = i + 11;
        HudMap_WriteTiles(2, y, left, 10);
        HudMap_WriteTiles(0x11, y, right, 10);
        right += 0x20;
        left += 0x20;
    }
    Menu_DrawHpMpBars(10, 4);
    HudMap_DrawStatusConditionIcon(0x14, 7);
    MenuPage_SetMode(entity, 0);
    ItemMenu_DrawList(menu, entity);
    Text12_ClearRegion();
    if (entity->unk_4F8.bytes[0] != 0)
    {
        const struct UsableItemInfo *info = &sConsumableInfo[MENU_PAGE_ITEM(entity)];
        s32 text = sItemDescriptionTextIds[info->name];

        MenuItemIcon_Show(0x1C, 0x94, 2, info->oneBasedIconId - 1);
        MenuItemIcon_SetHidden(0);
        MenuText_ClearAndPrint(text);
    }
    else
    {
        MenuItemIcon_SetHidden(1);
    }
    MenuPageTitle_Create(&gEwramData->entityData[20], 0x21);
    Cursor_SetTilePosition((MENU_PAGE_CURSOR(entity) & 1) ? 0x10 : 1,
                 ((entity->unk_4F8.bytes[1] - entity->unk_4F8.bytes[2]) >> 1) + 11);
    Cursor_Draw();
}
