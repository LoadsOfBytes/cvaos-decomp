/* Shop_CanTrade; records as
 * in the Shop_BuyItem draft. */
#include "types.h"
#include "syscalls.h"

const void *Item_GetInfo(s32 category, s32 index);
void *Item_GetInfoByGlobalId(s32 id);
u32 Inventory_GetOwnedCount(s32 category, s32 index);

/* Shop stock entries (item type, id; type 0xFF = id via Item_GetInfoByGlobalId). */
struct ShopStockEntry {
    u8 type;
    u8 id;
};

extern const struct ShopStockEntry sShopStock[];

/* Item info records; only the price at 4 is read here. */
struct ShopItemPriceView {
    u32 unk_0;
    u32 price;
};

struct ShopTransactionStats {
    u8 pad_0[0xF];
    u8 equippedYellowSoul;
    u8 pad_10[0x34 - 0x10];
    u32 money;
};

/* Shop menu state; per-row quantity at 0x3D1 + row. */
struct ShopQuantityOwner {
    u8 pad_0[0x3D1];
    u8 rowQuantities[1];
};

/**
 * @brief Return 1 when the deal is allowed: selling keeps the gold at most
 * 999999, buying keeps the count at most 9 and the gold covers the price
 * (8/10 with the discount).
 */
s32 Shop_CanTrade(struct ShopQuantityOwner *owner, u8 *menu, struct ShopTransactionStats *stats, s32 index)
{
    const struct ShopStockEntry *entry = &sShopStock[index];
    s32 owned = Inventory_GetOwnedCount(entry->type, entry->id);
    const struct ShopItemPriceView *item;
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
