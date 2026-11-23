/*
 * Shop: item list input, equipment preview, list drawing, sell list, buy / sell, trade checks, the confirm window and the list update.
 */
#include "types.h"
#include "gba.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "core.h"
#include "bg_and_collision.h"
#include "structs/main.h"
#include "structs/equipment_stats.h"
#include "gameplay.h"
#include "blue_souls.h"
#include "equipment_menu.h"
#include "menu_common.h"
#include "pause_menu.h"
#include "soul_menu.h"
#include "soul_pickup.h"
#include "text12.h"
#include "shop.h"
#include "data/data_4F1444.h"

void PlaySong(u32 song);
u32 Inventory_GetOwnedCount(s32 category, s32 index);

struct ShopStockEntry {
    u8 type;
    u8 id;
    u8 pad_2[2];
};
extern const struct ShopStockEntry sShopStock[];

struct Owner {
    u8 pad_0[0x3D1];
    u8 rowQuantities[1];
};

struct ShopMenu {
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B[0x14 - 0xB];
    s8 count;
    s8 visible;
    s8 top;
    u8 cursor;
};

/**
 * @brief Move the list cursor or change the quantity; return 0 nothing, 1
 * quantity changed, 2 cursor moved, 3 list scrolled.
 */
s32 Shop_HandleListInput(struct Owner *owner, struct ShopMenu *menu, void *unused, u8 *list, s32 input)
{
    const struct ShopStockEntry *entry;
    s32 owned;
    s32 limit;
    u8 *quantity;
    s32 top;
    s32 ret;

    list++;
    entry = &sShopStock[list[menu->cursor]];
    owned = Inventory_GetOwnedCount(entry->type, entry->id);
    limit = owned;
    quantity = &owner->rowQuantities[menu->cursor];
    top = menu->top;
    ret = 0;

    if ((input & 0x10) && *quantity != 0)
    {
        if (menu->state == 2)
        {
            if (*quantity < owned)
            {
                (*quantity)++;
                ret = 1;
            }
        }
        else if (*quantity + limit <= 8)
        {
            (*quantity)++;
            ret = 1;
        }
    }
    else if ((input & 0x20) && *quantity > 1)
    {
        (*quantity)--;
        ret = 1;
    }
    else if (input & 0x40)
    {
        if (menu->cursor != 0)
        {
            menu->cursor--;
            if (menu->top > menu->cursor)
                menu->top = menu->cursor;
            ret = 2;
        }
    }
    else if (input & 0x80)
    {
        if (menu->cursor < menu->count - 1)
        {
            menu->cursor++;
            if (menu->top < menu->cursor - 3)
                menu->top = menu->cursor - 3;
            ret = 2;
        }
    }
    else if (input & 0x100)
    {
        if (menu->cursor < menu->count - 1)
        {
            menu->top += 4;
            menu->cursor += 4;
            if (menu->top > menu->count - 4)
            {
                menu->top = menu->count - 4;
                menu->cursor = menu->count - 1;
            }
            ret = 2;
        }
    }
    else if (input & 0x200)
    {
        if (menu->cursor != 0)
        {
            if (menu->top <= 3)
            {
                menu->top = 0;
                menu->cursor = 0;
            }
            else
            {
                menu->top -= 4;
                menu->cursor -= 4;
            }
            ret = 2;
        }
    }
    menu->visible = menu->cursor - menu->top;
    if (menu->visible >= menu->count && menu->count != 0)
    {
        menu->cursor = menu->visible = menu->count - 1;
        ret = 3;
    }
    if (menu->top != top)
        ret = 3;
    if (ret != 0)
        PlaySong(0xF1);
    return ret;
}

extern const struct ArmorInfo sArmorInfo[];

extern s32 HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 color, s32 flag);

/**
 * @brief Preview the stat changes of equipping weapon/armor/accessory id,
 * then restore the equipment and recompute the stats.
 */
void Shop_PreviewEquipStats(void *unused, s32 type, s32 id)
{
    s16 *stats;
    s16 old[6];
    u8 saved[2];
    u8 savedWeapon;
    u8 *weapon;
    u8 *armor;
    u8 *accessory;
    s32 i;
    s16 kind;
    s32 color;
    s32 value;
    s16 diff;

    stats = &gEwramData->playerState.currentATK;
    for (i = 0; i < 6; i++)
        old[i] = stats[i];
    weapon = &gEwramData->playerStatus.equippedWeapon;
    savedWeapon = *weapon;
    armor = &gEwramData->playerStatus.equippedArmor;
    saved[0] = *armor;
    accessory = &gEwramData->playerStatus.equippedAccessory;
    saved[1] = *accessory;
    switch (type)
    {
    case 3:
        *weapon = id;
        break;
    case 4:
        if (sArmorInfo[id].type == 0)
            *armor = id;
        else
            *accessory = id;
        break;
    }
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    for (i = 0; i < 6; i++)
    {
        diff = stats[i] - old[i];
        if (diff == 0)
        {
            kind = 0;
            color = 3;
        }
        else if (diff > 0)
        {
            kind = 1;
            color = 9;
        }
        else
        {
            kind = 2;
            color = 8;
        }
        value = stats[i];
        if (value > 1000)
            value = 999;
        HudMap_DrawStatChangeIndicator(0x18, i + 2, kind);
        HudMap_PrintNumber(0x1B, i + 2, value, 3, color, 0);
    }
    gEwramData->playerStatus.equippedWeapon = savedWeapon;
    gEwramData->playerStatus.equippedArmor = saved[0];
    gEwramData->playerStatus.equippedAccessory = saved[1];
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
}

/* Shop_DrawList. */
u32 Inventory_GetOwnedCount(s32 category, s32 index);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);

/* Shop stock entries (item type, id; type 0xFF = id via Item_GetInfoByGlobalId). */

/* Item info records by type; only the name index at 0 is read here. */

extern const struct ConsumableInfo sConsumableInfo[];
extern const struct WeaponInfo sWeaponInfo[];

struct ShopItemInfo {
    u32 unk_0;
    s32 price;
};

struct ShopStats {
    u8 pad_0[0xF];
    u8 equippedYellowSoul;
};

/* Shop menu state; per-row quantity at 0x3D1 + row. */
struct ShopOwner {
    u8 pad_0[0x3D1];
    u8 rowQuantities[1];
};

struct ShopMenu2 {
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B[0x14 - 0xB];
    s8 count;
    u8 windowRow;
    s8 scroll;
    u8 cursor;
    u16 unk_18[6];
};

struct ShopList {
    u8 count;
    u8 items[1];
};
extern struct ShopList *const sShopLists[];

static inline u8 ShopItemAt(u8 *items, s32 row)
{
    return items[row];
}

/* Name index of a stock entry. A macro: the read has to be in every arm
 * (only the final load is shared by cross-jumping). */
#define SHOP_ENTRY_NAME(name, entry) \
    switch ((entry)->type) \
    { \
    case 3: \
        name = sWeaponInfo[(entry)->id].name; \
        break; \
    case 4: \
        name = sArmorInfo[(entry)->id].name; \
        break; \
    case 2: \
    default: \
        name = sConsumableInfo[(entry)->id].name; \
        break; \
    }

/**
 * @brief Draw the visible list rows and the selected entry details (owned
 * count, description, icon, stat preview) and the scroll arrows.
 */
void Shop_DrawList(struct ShopOwner *owner, struct ShopMenu2 *menu, struct ShopStats *stats, struct ShopList *list, s32 mode)
{
    s32 i;
    s32 row;
    s32 y;

    if (menu->scroll > list->count - 4)
    {
        menu->scroll = list->count - 4;
        if (menu->scroll < 0)
            menu->scroll = 0;
    }
    for (i = 0, y = 0xB; i < 4; y++, i++)
    {
        row = menu->scroll + i;
        if (mode > 2)
        {
            const struct ShopStockEntry *entry = &sShopStock[ShopItemAt(list->items, row)];
            u16 name;

            SHOP_ENTRY_NAME(name, entry);
            Text8_SetRegion(0, i + 0xC, 0xF, 1);
            Text8_ClearRegion();
            if (i >= list->count)
            {
                if (i == 0 && list->count == 0)
                    Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
            }
            else
            {
                Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[name]), 0);
            }
        }
        if (i >= list->count)
        {
            HudMap_ZeroTiles(0xE, y, 3);
            HudMap_ZeroTiles(0x15, y, 8);
        }
        else
        {
            const struct ShopItemInfo *item;
            const struct ShopStockEntry *entry;
            s32 price;
            s32 y2;

            HudMap_PrintPaddedNumber(0xF, y, owner->rowQuantities[row], 1);
            entry = &sShopStock[ShopItemAt(list->items, row)];
            {
                u8 type = entry->type;
                u8 id = entry->id;
                y2 = y;
                if (type != 0xFF)
                    item = Item_GetInfo(type, id);
                else
                    item = Item_GetInfoByGlobalId(id);
            }
            price = item->price;
            if (menu->state == 2)
                price /= 2;
            else if (stats->equippedYellowSoul == 9)
                price = Div(price * 8, 10);
            HudMap_ZeroTiles(0x15, y2, 8 - HudMap_PrintPaddedNumber(0x1C, y2, owner->rowQuantities[row] * price, 1));
        }
    }
    if (list->count == 0)
    {
        HudMap_ClearTilesLeft(0xC, 6, 2);
        EntityDelete((struct EwramData_EntityData *)((u8 *)gEwramData + 0xEB0));
        Text12_ClearRegion();
        for (i = 0; i < 6; i++)
        {
            HudMap_ZeroTiles(0x18, i + 2, 4);
            menu->unk_18[i] = 0x7FFF;
        }
        return;
    }
    row = menu->cursor = menu->windowRow + menu->scroll;
    menu->count = list->count;
    {
        const struct ShopStockEntry *entry = &sShopStock[ShopItemAt(list->items, row)];
        HudMap_PrintPaddedNumber(0xC, 6, Inventory_GetOwnedCount(entry->type, entry->id), 1);
    }
    if (mode > 1)
    {
        const struct ShopStockEntry *entry = &sShopStock[ShopItemAt(list->items, row)];
        u16 name;

        SHOP_ENTRY_NAME(name, entry);
        Text12_ClearRegion();
        Text12_PrintTwoLineEntryAt(sItemDescriptionTextIds[name], 2);
        {
            const struct ShopStockEntry *entry = &sShopStock[ShopItemAt(list->items, row)];
            MenuItemIcon_Show(0x1C, 0x90, entry->type, entry->id);
        }
        {
            const struct ShopStockEntry *entry = &sShopStock[ShopItemAt(list->items, row)];
            if (entry->type != 2)
            {
                Shop_PreviewEquipStats(menu, entry->type, entry->id);
            }
            else
            {
                for (i = 0; i < 6; i++)
                {
                    HudMap_ZeroTiles(0x18, i + 2, 4);
                    menu->unk_18[i] = 0x7FFF;
                }
            }
        }
    }
    {
        s32 tile = 0x334D;
        s32 x = 0x1D;
        s32 y2 = 0xB;

        if (menu->scroll > 0)
            HudMap_WriteTiles(x, y2, tile, 1);
        else
            HudMap_ClearTiles(x, y2, 1);
        if (menu->count - 4 > menu->scroll)
            HudMap_WriteTiles(x, y2 + 3, tile + 1, 1);
        else
            HudMap_ClearTiles(x, y2 + 3, 1);
    }
}

u32 Inventory_GetOwnedCount(s32 category, s32 index);
extern void *memset(void *dst, int value, unsigned long size);

struct SellItem {
    u32 unk_0;
    u32 price;
};

struct Owner2 {
    u8 pad_0[0x3D1];
    u8 rowQuantities[0x80];
};

struct SellList {
    u8 count;
    u8 items[0x87];
};

#define SELL_LIST ((struct SellList *)&gEwramData->sharedScratchBuffer[0x200])

static inline s32 FindStockEntry(s32 type, s32 id)
{
    s32 i;
    const struct ShopStockEntry *e = sShopStock;

    for (i = 0; i <= 0x7F; i++, e++)
    {
        if (e->id == id && e->type == type)
            return i;
    }
    return -1;
}

/**
 * @brief Build the sell list from every owned item with a price and a stock
 * entry in sShopStock, resetting quantities to 1.
 */
void Shop_BuildSellList(struct Owner2 *owner, void *unusedMenu, void *unusedStats)
{
    struct SellList *list;
    s32 i;
    s32 type;
    s32 id;
    struct SellItem *item;
    struct EwramData *ram;
    u8 owned;
    s32 ok;
    s32 index;

    list = SELL_LIST;
    list->count = 0;
    for (i = 0; i <= 0x87; i++)
    {
        type = Item_GetTypeByGlobalId(i);
        id = Item_GetIndexByGlobalId(i);
        item = Item_GetInfoByGlobalId(i);
        ram = gEwramData;
        Item_GetInfo(type, id);
        switch (type)
        {
        case 2:
            owned = ram->playerStatus.itemInventory[id];
            break;
        case 3:
            owned = ram->playerStatus.weaponInventory[id];
            break;
        case 4:
            owned = ram->playerStatus.armorInventory[id];
            break;
        }
        ok = owned != 0 && item->price != 0;
        if (ok)
        {
            index = FindStockEntry(Item_GetTypeByGlobalId(i), Item_GetIndexByGlobalId(i));
            if (index >= 0)
                list->items[list->count++] = index;
        }
    }
    list = SELL_LIST;
    memset(owner->rowQuantities, 0, 0x80);
    for (i = 0; i < list->count; i++)
    {
        const struct ShopStockEntry *entry = &sShopStock[list->items[i]];
        owner->rowQuantities[i] = Inventory_GetOwnedCount(entry->type, entry->id) != 0;
    }
}

/* Shop_SellItem: sell the quantity of row param_1[0x17] of shop stock entry
 * index (records as in this file): lower the owned count (types 3/4
 * clear an equipped slot that drops to zero), add half price times the
 * quantity to the gold and refresh the row. */

u32 Inventory_GetOwnedCount(s32 category, s32 index);

struct Item {
    u32 unk_0;
    s32 price;
    u8 armorSlotKind;
};

struct Shop {
    u8 pad_0[0xC];
    u8 equippedWeapon;
    u8 pad_D[0x10 - 0xD];
    u8 equippedArmorAndAccessory[2];
    u8 pad_12[0x34 - 0x12];
    s32 money;
    u8 itemInventory[0x20];
    u8 weaponInventory[0x3B];
    u8 armorAndAccessoryInventory[1];
};

/**
 * @brief Sell the selected quantity (unequipping when needed), add the gold
 * and rebuild the sell list.
 */
void Shop_SellItem(struct Owner *owner, u8 *menu, struct Shop *stats, s32 index)
{
    const struct ShopStockEntry *entry = &sShopStock[index];
    u8 count = owner->rowQuantities[menu[0x17]];
    u8 kind = ((const struct Item *)Item_GetInfo(entry->type, entry->id))->armorSlotKind;
    const struct Item *item;
    s32 n;

    switch (entry->type)
    {
    case 3:
        n = stats->weaponInventory[entry->id];
        n -= count;
        if (n <= 0)
        {
            if (stats->equippedWeapon == entry->id)
            {
                stats->equippedWeapon = 0xFF;
                Player_RefreshEquipmentStats();
            }
            n = 0;
        }
        stats->weaponInventory[entry->id] = n;
        break;
    case 4:
    {
        s32 slot = kind != 0;
        n = stats->armorAndAccessoryInventory[entry->id];
        n -= count;
        if (n <= 0)
        {
            if (stats->equippedArmorAndAccessory[slot] == entry->id)
            {
                stats->equippedArmorAndAccessory[slot] = 0xFF;
                Player_RefreshEquipmentStats();
            }
            n = 0;
        }
        stats->armorAndAccessoryInventory[entry->id] = n;
        break;
    }
    default:
        stats->itemInventory[entry->id] -= count;
        break;
    }
    {
        u8 type = entry->type;
        u8 id = entry->id;
        if (type != 0xFF)
            item = Item_GetInfo(type, id);
        else
            item = Item_GetInfoByGlobalId(id);
    }
    stats->money += item->price / 2 * count;
    {
        const struct ShopStockEntry *e = &sShopStock[index];
        owner->rowQuantities[menu[0x17]] = Inventory_GetOwnedCount(e->type, e->id) != 0;
    }
    Shop_BuildSellList((struct Owner2 *)owner, menu, stats);
}

struct ShopItemPriceView {
    u32 unk_0;
    s32 price;
};

struct ShopTransactionStats {
    u8 pad_0[0xF];
    u8 equippedYellowSoul;
    u8 pad_10[0x34 - 0x10];
    s32 money;
    u8 itemInventory[0x20];
    u8 weaponInventory[0x3B];
    u8 armorAndAccessoryInventory[1];
};

struct ShopQuantityOwner {
    u8 pad_0[0x3D1];
    u8 rowQuantities[1];
};

/* Buy the selected row quantity and reset that row's quantity to one.
 * Keep the selected inventory pointer in r1 through the increment; agbcc
 * otherwise swaps its base and item-id registers. This constraint does
 * not establish the original source spelling or organization. */
/**
 * @brief Buy the selected quantity, pay its price and reset the row quantity
 * to one.
 */
void Shop_BuyItem(struct ShopQuantityOwner *owner, u8 *menu, struct ShopTransactionStats *stats, s32 index)
{
    const struct ShopStockEntry *entry = &sShopStock[index];
    s32 count = owner->rowQuantities[menu[0x17]];
    register u8 *slot asm("r1"); /* FAKEMATCH: pinned register */
    s32 price;

    switch (entry->type)
    {
    case 3:
        slot = stats->weaponInventory;
        break;
    case 4:
        slot = stats->armorAndAccessoryInventory;
        break;
    default:
        slot = stats->itemInventory;
        break;
    }
    slot += entry->id;
    *slot += count;
    {
        u8 type = entry->type;
        u8 id = entry->id;
        if (type != 0xFF)
            price = ((const struct ShopItemPriceView *)Item_GetInfo(type, id))->price;
        else
            price = ((const struct ShopItemPriceView *)Item_GetInfoByGlobalId(id))->price;
    }
    if (stats->equippedYellowSoul == 9)
        price = Div(price * 8, 10);
    stats->money -= count * price;
    owner->rowQuantities[menu[0x17]] = 1;
}

/* Shop_CanTrade; records as
 * in the Shop_BuyItem draft. */

u32 Inventory_GetOwnedCount(s32 category, s32 index);

/* Shop stock entries (item type, id; type 0xFF = id via Item_GetInfoByGlobalId). */

/* Item info records; only the price at 4 is read here. */
struct ShopItemPriceView2 {
    u32 unk_0;
    u32 price;
};

struct ShopTransactionStats2 {
    u8 pad_0[0xF];
    u8 equippedYellowSoul;
    u8 pad_10[0x34 - 0x10];
    u32 money;
};

/* Shop menu state; per-row quantity at 0x3D1 + row. */

/**
 * @brief Return 1 when the deal is allowed: selling keeps the gold at most
 * 999999, buying keeps the count at most 9 and the gold covers the price
 * (8/10 with the discount).
 */
s32 Shop_CanTrade(struct ShopQuantityOwner *owner, u8 *menu, struct ShopTransactionStats2 *stats, s32 index)
{
    const struct ShopStockEntry *entry = &sShopStock[index];
    s32 owned = Inventory_GetOwnedCount(entry->type, entry->id);
    const struct ShopItemPriceView2 *item;
    u32 price;

    {
        u8 type = entry->type;
        u8 id = entry->id;
        if (type != 0xFF)
            item = Item_GetInfo(type, id);
        else
            item = Item_GetInfoByGlobalId(id);
    }
    price = item->price;
    if (menu[0xA] == 2)
    {
        u8 count;
        price >>= 1;
        count = owner->rowQuantities[menu[0x17]];
        if (stats->money + price * count <= 999999 && count != 0)
            return 1;
    }
    else if (owned + owner->rowQuantities[menu[0x17]] <= 9)
    {
        if (stats->equippedYellowSoul == 9)
            price = Div(price * 8, 10);
        if (stats->money >= price * owner->rowQuantities[menu[0x17]])
            return 1;
    }
    return 0;
}

/* Shop_ShowConfirmWindow: wavy HBlank BG1 vertical offset table (rows 1..63 = 0x40) and windows 0/1 setup. */
/**
 * @brief Open the yes/no confirmation: BG1 H-blank offsets and windows 0/1.
 */
void Shop_ShowConfirmWindow(void)
{
    s16 *buf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    s16 *rows = &buf[0x60];
    s32 i;

    HBlankEffectSetup(0xA0, 0xA0, 2, (void *)REG_BG1VOFS);
    { s16 v = 0x40; s32 n = 0x40; s16 *p = &rows[1]; for (i = 1; i < n; i++) *p++ = v; }
    rows[i] = 0;
    gEwramData->hBlankEffect.requestStart = 1;
    gUnk_03002CB0.dispCnt |= DCNT_WIN0;
    {
        struct DisplayRegisters *dr = &gDisplayRegisters;
        u8 *out = &dr->winOut_L;
        u8 v = 0x3F;
        dr->winIn_L = 0x3E;
        dr->win0H = 0x28F0;
        gDisplayRegisters.win0V = 0x7898;
        gUnk_03002CB0.dispCnt |= DCNT_WIN1;
        *out = v;
        dr->winIn_H = 0x2F;
        dr->win1H = 0x28;
        dr->win1V = 0x7898;
    }
}

/* Shop menu page setup (Shop_OpenList). The flag loop has its own index
 * (j) so it stays in a caller-saved register. */

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B[0x14 - 0xB];
    u8 count;
    u8 cursor;
    u8 top;
    u8 item;
};

static inline u8 ShopMode(u8 *s) { return s[0x3D0]; }
static inline u8 *ShopFlags(u8 *s) { return s + 0x3D1; }

/**
 * @brief Set up the item list page for buying or selling (building the sell
 * list) and draw it.
 */
void Shop_OpenList(u8 *shop, struct MenuPage *page)
{
    struct PlayerStatus *stats;
    s32 i;
    s32 j;
    u8 *mode;
    u8 *list;

    stats = &gEwramData->playerStatus;
    DMA_FILL_32(3, 0, 0x0600E000, 0x800);
    HudMap_PrintNumber(0x11, 9, stats->currentGold, 6, 6, 0);
    Menu_DrawAttackDefense(0x16, 2);
    Menu_DrawCurrentStats(0x16, 4);
    for (i = 0; i <= 3; i++)
    {
        HudMap_WriteTiles(5, i + 0x10, (i << 5) - 0xFA0, 0x18);
        HudMap_WriteTiles(2, i + 0xB, (i << 5) - 0xE80, 0xA);
    }
    Text12_SetRegion(0, 2, 0x23, 5);
    Text8_SetRegion(0, 0xC, 0x20, 4);
    Text8_ClearRegion();
    MenuPage_SetMode((struct EwramData_EntityData *)page, 0);
    page->top = 0;
    page->count = 0;
    page->item = 0;
    page->cursor = 0;
    mode = &shop[0x3D0];
    *mode = 0;
    if (gEwramData->gameState.bossEventFlags & 0x10)
        *mode = 1;
    if (gEwramData->gameState.bossEventFlags & 2)
        *mode = 2;
    if (page->state == 2)
    {
        Shop_BuildSellList((struct Owner2 *)shop, page, stats);
        Shop_DrawList((struct ShopOwner *)shop, (struct ShopMenu2 *)page, (struct ShopStats *)stats, (struct ShopList *)((u8 *)gEwramData + 0x135F4), 3);
    }
    else
    {
        list = (u8 *)sShopLists[ShopMode(shop)];
        for (j = 0; j < list[0]; j++)
        {
            u8 *p = ShopFlags(shop);
            p[j] = 1;
        }
        Shop_DrawList((struct ShopOwner *)shop, (struct ShopMenu2 *)page, (struct ShopStats *)stats, (struct ShopList *)list, 3);
    }
}

struct ShopList;
void PlaySong(u32 song);

struct Owner3 {
    u8 pad_0[0x3D0];
    u8 shop;
};

struct ShopMenu3 {
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B;
    u8 stage;
    u8 timer;
    u8 pad_E[0x17 - 0xE];
    u8 cursor;
};

#undef SELL_LIST
#define SELL_LIST ((struct ShopList *)&gEwramData->sharedScratchBuffer[0x200])

static inline u8 ShopItem(u8 *items, struct ShopMenu3 *menu)
{
    return items[menu->cursor];
}

/**
 * @brief Handle the item list and the yes/no confirmation; sell or buy the
 * selected item on yes. Return nonzero when the list is closed.
 */
s32 Shop_UpdateList(struct Owner3 *owner, struct ShopMenu3 *menu)
{
    struct PlayerStatus *stats = &gEwramData->playerStatus;
    s32 input = gEwramData->inputData.repeatedInput;
    s32 ret = 0;
    struct ShopList *list;
    s32 result;

    if (menu->state == 2)
        list = SELL_LIST;
    else
        list = sShopLists[owner->shop];
    if (menu->stage == 0)
    {
        if (input & 2)
        {
            PlaySong(0xF0);
            ret = 1;
        }
        else if (input & 1)
        {
            if (Shop_CanTrade((struct ShopQuantityOwner *)owner, (u8 *)menu, (struct ShopTransactionStats2 *)stats, ShopItem(list->items, menu)))
            {
                Shop_ShowConfirmWindow();
                PlaySong(0xF3);
                menu->timer = 0;
                menu->stage++;
            }
            else
            {
                PlaySong(0xEF);
            }
        }
        else
        {
            result = Shop_HandleListInput((struct Owner *)owner, (struct ShopMenu *)menu, stats, (u8 *)list, input);
            if (result != 0)
                Shop_DrawList((struct ShopOwner *)owner, (struct ShopMenu2 *)menu, (struct ShopStats *)stats, list, result);
        }
    }
    else if ((input & 1) && menu->timer == 0)
    {
        if (menu->state == 2)
            Shop_SellItem((struct Owner *)owner, (u8 *)menu, (struct Shop *)stats, ShopItem(list->items, menu));
        else
            Shop_BuyItem((struct ShopQuantityOwner *)owner, (u8 *)menu, (struct ShopTransactionStats *)stats, ShopItem(list->items, menu));
        PlaySong(0xF5);
        Shop_DrawList((struct ShopOwner *)owner, (struct ShopMenu2 *)menu, (struct ShopStats *)stats, list, 3);
        HudMap_ZeroTiles(0xA, 3, 9 - HudMap_PrintNumber(0x11, 9, stats->currentGold, 6, 6, 0));
        gEwramData->hBlankEffect.requestStop = 1;
        gUnk_03002CB0.dispCnt &= ~0x2000;
        gUnk_03002CB0.dispCnt &= ~0x4000;
        menu->stage = 0;
    }
    else if (input & 3)
    {
        PlaySong(0xF0);
        gEwramData->hBlankEffect.requestStop = 1;
        gUnk_03002CB0.dispCnt &= ~0x2000;
        gUnk_03002CB0.dispCnt &= ~0x4000;
        menu->stage = 0;
    }
    else if ((input & 0x10) && menu->timer == 0)
    {
        PlaySong(0xF1);
        menu->timer++;
    }
    else if ((input & 0x20) && menu->timer != 0)
    {
        PlaySong(0xF1);
        menu->timer = 0;
    }
    return ret;
}

/* Read the inventory count for consumables, weapons or armour. */
/**
 * @brief Return the owned count of a consumable, weapon or armor (types 2..4).
 */
u32 Inventory_GetOwnedCount(s32 category,s32 index)
{
    struct EwramData *ram=gEwramData;
    u32 result=(u32)Item_GetInfo(category,index);
    switch(category)
    {
    case 2:return ram->playerStatus.itemInventory[index];
    case 3:return ram->playerStatus.weaponInventory[index];
    case 4:return ram->playerStatus.armorInventory[index];
    }
    return result;
}
