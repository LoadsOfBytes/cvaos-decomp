/* Shop_SellItem: sell the quantity of row param_1[0x17] of shop stock entry
 * index (records as in code_080671A4.c): lower the owned count (types 3/4
 * clear an equipped slot that drops to zero), add half price times the
 * quantity to the gold and refresh the row. */
#include "types.h"
#include "syscalls.h"

const void *Item_GetInfo(s32 category, s32 index);
void *Item_GetInfoByGlobalId(s32 id);
u32 Inventory_GetOwnedCount(s32 category, s32 index);
extern void Player_RefreshEquipmentStats(void);
extern void Shop_BuildSellList(void *, u8 *, void *);

struct ShopStockEntry {
    u8 type;
    u8 id;
};
extern const struct ShopStockEntry sShopStock[];

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

struct Owner {
    u8 pad_0[0x3D1];
    u8 rowQuantities[1];
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
    Shop_BuildSellList(owner, menu, stats);
}
