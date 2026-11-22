#include "gba.h"
#include "structs/ewram.h"

const void *Item_GetInfo(s32 category, s32 index);
void *Item_GetInfoByGlobalId(s32 id);
s32 Item_GetTypeByGlobalId(s32 index);
s32 Item_GetIndexByGlobalId(s32 index);
u32 Inventory_GetOwnedCount(s32 category, s32 index);
extern void *memset(void *dst, int value, unsigned long size);

struct ShopStockEntry {
    u8 type;
    u8 id;
    u8 pad_2[2];
};
extern const struct ShopStockEntry sShopStock[];

struct SellItem {
    u32 unk_0;
    u32 price;
};

struct Owner {
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
void Shop_BuildSellList(struct Owner *owner)
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
