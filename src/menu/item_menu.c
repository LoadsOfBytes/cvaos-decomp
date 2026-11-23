/*
 * Equipment list update and the item menu.
 */
#include "item_menu.h"
#include "gameplay.h"
#include "sound.h"
#include "structs/ewram.h"
#include "structs/equipment_stats.h"
#include "types.h"
#include "player_core.h"
#include "menu_entity.h"
#include "blue_souls.h"
#include "equipment_list_update.h"
#include "equipment_menu.h"
#include "menu_common.h"
#include "pause_menu.h"
#include "soul_menu.h"
#include "soul_pickup.h"
#include "text12.h"

/* Equipment menu input handler: A equips the highlighted item (weapon or
 * armor/accessory slot unk_B - 1), A/B closes the list and redraws the page,
 * cursor moves refresh the item description. */

extern void MenuItemIcon_SetHidden(u8 value);

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 pad_D[0x14 - 0xD];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
    u8 pad_18[0x39 - 0x18];
    s8 drawnTop;
};

struct ItemInfo { u16 name; };

extern struct WeaponInfo sWeaponInfo[];
extern const struct ArmorInfo sArmorInfo[];
extern const u16 sItemDescriptionTextIds[];

/* Called with an int tile here (no u16 truncation in the original). */
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
extern void EquipmentMenu_DrawList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot);
extern void EquipmentMenu_ShowSlotDescription(struct PlayerStatus *stats, struct MenuPage *page);

extern void EquipmentMenu_CountList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot);
extern s32 MenuGrid_MoveCursor(void *page, s32 input);
extern void EquipmentMenu_DrawEquippedIcons(struct PlayerStatus *stats, struct MenuPage *page);

s32 EquipmentMenu_UpdateList(struct MenuPage *page)
{
    struct PlayerStatus *stats;
    s32 repeated;
    s32 new;
    s32 moved;
    s32 i;
    s32 old;
    s32 text;
    s32 type;
    const u8 *info;
    u8 *slots;

    stats = &gEwramData->playerStatus;
    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    if (page->stage == 0)
    {
        Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 2, ((page->cursor - page->top) >> 1) + 0xB);
        page->stage++;
    }
    if (new & 1)
    {
        PlaySong(0xF4);
        if (page->subState == 0)
        {
            if (stats->equippedWeapon != 0xFF)
                EquipmentList_UpdateForEquip(page);
            stats->equippedWeapon = page->item;
        }
        else
        {
            s32 slot = page->subState - 1;
            slots = &stats->equippedArmor;
            if (slots[slot] != 0xFF)
                EquipmentList_UpdateForEquip(page);
            {
                s32 slot2 = page->subState - 1;
                slots[slot2] = page->item;
            }
        }
        Player_RefreshEquipmentStats();
        YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    }
    moved = MenuGrid_MoveCursor(page, repeated);
    if (new & 3)
    {
        for (i = 0; i <= 6; i++)
            HudMap_ClearTiles(0x18, i + 2, 4);
        page->state = 2;
        for (i = 3; i >= 0; i--)
            HudMap_ClearTiles(0x19, i + 5, 3);
        EquipmentMenu_PrintEquippedNames();
        EquipmentMenu_ShowSlotDescription(stats, page);
        if (new & 1)
        {
            EquipmentMenu_DrawEquippedIcons(stats, page);
            PlaySong(0xF3);
        }
        else
        {
            PlaySong(0xF0);
        }
        Menu_DrawAttackDefense(0x16, 2);
        Menu_DrawCurrentStats(0x16, 5);
        if (page->cursor >= page->count)
        {
            page->cursor = page->count - 1;
            if (page->top > page->cursor)
                page->top = page->top - 2;
        }
        EquipmentMenu_DrawList(stats, page, page->subState);
        MenuList_DrawScrollArrows(page, 0);
    }
    else if (moved)
    {
        old = page->item;
        if (page->drawnTop != page->top)
        {
            page->drawnTop = page->top;
            EquipmentMenu_DrawList(stats, page, page->subState);
            MenuList_DrawScrollArrows(page, 0);
        }
        else
        {
            EquipmentMenu_CountList(&gEwramData->playerStatus, page, page->subState);
        }
        if (old != page->item)
        {
            Text12_ClearRegion();
            if (page->item != 0xFF)
            {
                if (page->subState == 0)
                {
                    info = (const u8 *)&sWeaponInfo[page->item];
                    type = 3;
                }
                else
                {
                    info = (const u8 *)&sArmorInfo[page->item];
                    type = 4;
                }
                text = sItemDescriptionTextIds[*(u16 *)info];
                MenuItemIcon_Show(0x1C, 0x94, type, page->item);
                MenuItemIcon_SetHidden(0);
            }
            else
            {
                text = 0x350;
                MenuItemIcon_SetHidden(1);
            }
            MenuText_ClearAndPrint(text);
            EquipmentMenu_DrawStatPreview(page);
        }
        Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 2, ((page->cursor - page->top) >> 1) + 0xB);
        PlaySong(0xF1);
    }
    Cursor_Draw();
    return 0;
}

/* Equipment stat preview and selection helpers. */

/**
 * @brief Prepare equipment bonuses using the supplied override and add base stats into the shared preview; do not apply it to current player stats.
 */
void Equipment_PrepareStatPreview(s32 category, s32 item)
{
    struct EwramData *ewram = gEwramData;
    struct EquipmentStats *preview = (struct EquipmentStats *)ewram->sharedScratchBuffer;
    struct PlayerStatus *player = &ewram->playerStatus;
    Equipment_PrepareBonuses(category, item);
    preview->stats[0] = preview->stats[0] + player->baseStats[0];
    preview->stats[1] = preview->stats[1] + player->baseStats[1];
    preview->stats[2] = preview->stats[2] + player->baseStats[2];
    preview->stats[3] = preview->stats[3] + player->baseStats[3];
}

/**
 * @brief Store the item in the selected equipment category, refresh equipment stats and restart the equipped yellow soul; retain unsupported-category behavior.
 */
void Player_EquipItem(s32 category, s32 item)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    switch (category)
    {
        case 2:
            player->equippedWeapon = item;
            break;
        case 0:
            player->equippedArmor = item;
            break;
        case 1:
            player->equippedAccessory = item;
            break;
    }
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
}

/* Store value into the equipment list of the given kind (0 armor,
 * 1 accessory, 2 weapon): only slot 0 when value is nonzero, otherwise all
 * four slots. Same shape as SoulList_WriteValue for the soul lists. */
/**
 * @brief Write the value to slot zero, returning if its low byte is nonzero; otherwise write all four entries and retain the final slot-three assignment.
 */
void EquipmentList_WriteValue(s32 kind, s32 value)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list;
    s32 i;

    switch (kind)
    {
        case 2:
            list = player->weaponList;
            break;
        case 0:
            list = player->armorList;
            break;
        case 1:
            list = player->accessoryList;
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

/* Equipped item of the given kind (0 armor, 1 accessory, 2 weapon). */
/**
 * @brief Return the equipped armor, accessory or weapon for categories zero, one or two; retain the unset result for unsupported categories.
 */
u8 Player_GetEquippedItem(s32 kind)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 item;

    switch (kind)
    {
        case 2:
            item = player->equippedWeapon;
            break;
        case 0:
            item = player->equippedArmor;
            break;
        case 1:
            item = player->equippedAccessory;
            break;
    }
    return item;
}

/* Convert a global item id to its type/index before updating the list. */

void EquipmentList_InsertGlobalItem(s32 item)
{
    s32 type = Item_GetTypeByGlobalId(item);
    s32 index = Item_GetIndexByGlobalId(item);
    EquipmentList_Insert(type, index);
}

extern const struct ConsumableInfo sConsumableInfo[];
static inline s32 Add16(s16 a, u16 b) { return a + b; }

/* Apply selected consumable: HP/MP recovery, status cure or HP cost. */
s32 ItemMenu_UseItem(struct PlayerStatus *stats, struct EwramData_EntityData *menu)
{
    s32 changed = 0;
    const struct ConsumableInfo *info;
    s32 effect;
    u16 amount, maxMP;
    if ((gEwramData->gameState.gameFlagsTail[0] & 0x100000) || (u8)menu->ext.menuPage.item == 0xFF)
        return 0;
    info = &sConsumableInfo[(u8)menu->ext.menuPage.item];
    effect = info->effect;
    if (effect == 3 && Player_TestSoulStateFlags(0x100000)) effect = 0;
    switch (effect)
    {
    case 0:
        if (stats->currentHP < stats->maxHP)
        {
            changed = 1;
            stats->currentHP += info->value;
            if (stats->currentHP > stats->maxHP) stats->currentHP = stats->maxHP;
        }
        break;
    case 1:
        maxMP = stats->maxMP;
        amount = maxMP / (100 / info->value);
        if (stats->currentMP < maxMP)
        {
            changed = 1;
            stats->currentMP = Add16(amount, stats->currentMP);
            if (stats->currentMP > maxMP) stats->currentMP = maxMP;
        }
        break;
    case 2:
        if (info->value & gEwramData->playerStatus.statusEffects)
        {
            changed = 1;
            Player_ClearStatusEffects(info->value);
        }
        break;
    case 3:
        if (stats->currentHP > 1)
        {
            changed = 1;
            stats->currentHP -= info->value;
            if (stats->currentHP <= 0) stats->currentHP = 1;
        }
        break;
    }
    if (effect == 3)
    {
        PlaySong(0x198);
        PlaySong(0xEF);
    }
    else PlaySong(0xF8);
    return changed;
}

/* Item list page. */
struct MenuPage2
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
};

extern const u16 sItemNameTextIds[];

/**
 * @brief 4B494 | Draw the item list page (32 items, 8 rows): name and count
 * of each owned item from the page top, blank rows, store the number of
 * entries and the item under the cursor. Declaration order i, idx, row
 * gives the original stack slot order of the loop temporaries.
 */
void ItemMenu_DrawList(struct PlayerStatus *stats, struct MenuPage2 *page)
{
    s32 i;
    s32 idx;
    s32 row;

    row = 0;
    idx = 0;
    page->item = 0xFF;
    for (i = 0; idx < page->top && i < 0x20; i++)
    {
        if (stats->itemInventory[i] != 0)
            idx++;
    }
    for (; row <= 7 && i < 0x20; i++)
    {
        if (stats->itemInventory[i] != 0)
        {
            const struct ConsumableInfo *info;
            s32 limit; /* never initialised, as in the original */
            s32 odd;
            u16 yy;
            s32 x;

            if (idx == page->cursor)
                page->item = i;
            info = &sConsumableInfo[i];
            while (1)
            {
                s32 v = *(vu16 *)0x04000006;
                if (v <= 0x40 || v >= limit)
                    break;
            }
            odd = row & 1;
            x = odd ? 0x15 : 0;
            yy = (row >> 1) + 0xC;
            Text8_SetRegion(x, yy, 0xE, 1);
            Text8_ClearRegion();
            Text8_SetPixelCursor((row & 1) << 7, yy);
            Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[info->name]), 0);
            HudMap_PrintPaddedNumber(odd ? 0x1C : 0xD, (row >> 1) + 0xB, stats->itemInventory[i], 1);
            idx++;
            row++;
        }
    }
    for (; row <= 7; row++)
    {
        s32 odd;
        s32 half;
        s32 x;

        odd = row & 1;
        x = odd ? 0x15 : 0;
        half = row >> 1;
        Text8_SetRegion(x, half + 0xC, 0xE, 1);
        Text8_ClearRegion();
        HudMap_ClearTilesLeft(odd ? 0x1C : 0xD, half + 0xB, 2);
    }
    for (; i < 0x20; i++)
    {
        if (stats->itemInventory[i] != 0)
            idx++;
    }
    page->count = idx;
    if (idx == 0)
    {
        Text8_SetRegion(0, 0xC, 0xA, 1);
        Text8_ClearRegion();
        Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
        page->top = idx;
        page->cursor = idx;
        page->item = 0xFF;
    }
    MenuList_DrawScrollArrows((struct MenuPage *)page, 0);
}

/* ItemMenu_CountList, ItemMenu_Open. */
/* Usable item records (16 bytes; see Item_GetInfo). */

extern void MenuPage_SetMode(struct EwramData_EntityData *entity, u8 mode);
extern void MenuPageTitle_Create(struct EwramData_EntityData *title, s32 frame);

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

    entity->ext.menuPage.item = 0xFF;
    i = 0;
    while (pos < entity->ext.menuPageList.bytes[2] && i <= 0x1F)
    {
        if (menu->itemInventory[i] != 0)
            pos++;
        i++;
    }
    while (shown <= 7 && i <= 0x1F)
    {
        if (menu->itemInventory[i] != 0)
        {
            if (pos == entity->ext.menuPageList.bytes[1])
                entity->ext.menuPage.item = i;
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
    entity->ext.menuPage.count = pos;
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

    entity->ext.menuPage.top = 0;
    entity->ext.menuPage.cursor = 0;
    entity->ext.menuPage.item = 0xFF;
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
    ItemMenu_DrawList(menu, (struct MenuPage2 *)entity);
    Text12_ClearRegion();
    if (entity->ext.menuPageList.bytes[0] != 0)
    {
        const struct ConsumableInfo *info = &sConsumableInfo[entity->ext.menuPage.item];
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
    Cursor_SetTilePosition((entity->ext.menuPage.cursor & 1) ? 0x10 : 1,
                 ((entity->ext.menuPageList.bytes[1] - entity->ext.menuPageList.bytes[2]) >> 1) + 11);
    Cursor_Draw();
}

struct MenuPage3
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
    u8 pad_18[0x39 - 0x18];
    s8 drawnTop;
};

static inline void ShowItemInfo(struct MenuPage3 *page)
{
    Text12_ClearRegion();
    if (page->count != 0)
    {
        const struct ConsumableInfo *info = &sConsumableInfo[page->item];
        s32 text = sItemDescriptionTextIds[info->name];

        MenuItemIcon_Show(0x1C, 0x94, 2, info->oneBasedIconId - 1);
        MenuItemIcon_SetHidden(0);
        MenuText_ClearAndPrint(text);
    }
    else
    {
        MenuItemIcon_SetHidden(1);
    }
}

/**
 * @brief 4B808 | Item page update: A uses the selected item (one fewer in
 * itemInventory, cursor moves up when the last entry ran out), cursor
 * movement updates the item info, B leaves.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 ItemMenu_Update(struct MenuPage3 *page)
{
    struct EwramData *ew;
    struct PlayerStatus *stats;
    s32 repeated;
    s32 new;
    s32 moved;
    s32 oldItem;

    ew = gEwramData;
    stats = &ew->playerStatus;
    repeated = ew->inputData.repeatedInput;
    new = ew->inputData.newInput;
    if (new & 1)
    {
        moved = ItemMenu_UseItem(stats, (struct EwramData_EntityData *)page);
        if (moved != 0)
        {
            ew->playerStatus.itemInventory[page->item]--;
            if (ew->playerStatus.itemInventory[page->item] == 0 && page->cursor == page->count - 1)
            {
                if (page->top != 0)
                {
                    if (page->cursor == page->top)
                        page->top -= 8;
                    page->cursor--;
                }
                else if (page->cursor != 0)
                {
                    page->cursor--;
                }
            }
            Menu_DrawHpMpBars(0xA, 4);
            HudMap_DrawStatusConditionIcon(0x14, 7);
            ItemMenu_DrawList(stats, (struct MenuPage2 *)page);
            ShowItemInfo(page);
        }
        else
        {
            PlaySong(0xEF);
        }
    }
    else
    {
        moved = MenuGrid_MoveCursor(page, repeated);
        if (new & 2)
        {
            MenuItemIcon_Delete();
            MenuPageTitle_Delete(&gEwramData->entityData[20]);
            PlaySong(0xF0);
            return -1;
        }
        if (moved)
            PlaySong(0xF1);
    }
    if (moved)
    {
        oldItem = page->item;
        if (page->drawnTop != page->top)
        {
            page->drawnTop = page->top;
            ItemMenu_DrawList(stats, (struct MenuPage2 *)page);
        }
        else
        {
            ItemMenu_CountList(&gEwramData->playerStatus, (struct EwramData_EntityData *)page);
        }
        if (oldItem != page->item)
            ShowItemInfo(page);
    }
    Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 1, ((page->cursor - page->top) >> 1) + 0xB);
    Cursor_Draw();
    MenuPageTitle_Update(&gEwramData->entityData[20]);
    return (new & 2) ? -1 : 0;
}
