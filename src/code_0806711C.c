#include "types.h"
#include "syscalls.h"

const void *Item_GetInfo(s32 category, s32 index);
void *Item_GetInfoByGlobalId(s32 id);

struct ShopStockEntry {
    u8 type;
    u8 id;
};

extern const struct ShopStockEntry sShopStock[];

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
