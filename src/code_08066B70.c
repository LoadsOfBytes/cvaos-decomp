/* Shop_DrawList. */
#include "gba.h"
#include "syscalls.h"
#include "code_08009A0.h"
#include "structs/ewram.h"

extern void Text8_SetRegion(u16 left, u16 top, u16 width, u16 height);
extern void Text8_ClearRegion(void);
extern u8 *Text_GetLocalizedEntry(s32 index);
extern s32 Text8_Print(void *text, s32 index);
extern void HudMap_ZeroTiles(s32 x, s32 y, s32 count);
extern s32 HudMap_PrintPaddedNumber(s32 x, s32 y, s32 value, s32 digits);
const void *Item_GetInfo(s32 category, s32 index);
void *Item_GetInfoByGlobalId(s32 id);
extern void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count);
extern void Text12_ClearRegion(void);
u32 Inventory_GetOwnedCount(s32 category, s32 index);
extern void Text12_PrintTwoLineEntryAt(s32 index, s32 row);
extern void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item);
extern void Shop_PreviewEquipStats(void *unused, s32 type, s32 id);
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);

/* Shop stock entries (item type, id; type 0xFF = id via Item_GetInfoByGlobalId). */
struct ShopStockEntry {
    u8 type;
    u8 id;
    u8 pad_2[2];
};

extern const struct ShopStockEntry sShopStock[];

/* Item info records by type; only the name index at 0 is read here. */
struct ShopItemName { u16 name; u8 pad_2[0x10 - 2]; };
struct ShopWeaponName { u16 name; u8 pad_2[0x1C - 2]; };
struct ShopArmorName { u16 name; u8 pad_2[0x14 - 2]; };

extern const struct ShopItemName sConsumableInfo[];
extern const struct ShopWeaponName sWeaponInfo[];
extern const struct ShopArmorName sArmorInfo[];
extern const u16 sItemNameTextIds[]; /* name text ids */
extern const u16 sItemDescriptionTextIds[]; /* description text ids */

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

struct ShopMenu {
    u8 pad_0[0xA];
    u8 state;
    u8 pad_B[0x14 - 0xB];
    s8 count;
    u8 unk_15;
    s8 scroll;
    u8 cursor;
    u16 unk_18[6];
};

struct ShopList {
    u8 count;
    u8 items[1];
};

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
void Shop_DrawList(struct ShopOwner *owner, struct ShopMenu *menu, struct ShopStats *stats, struct ShopList *list, s32 mode)
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
    row = menu->cursor = menu->unk_15 + menu->scroll;
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
