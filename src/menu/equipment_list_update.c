/* Update a four-entry equipment list before replacing the equipped item.
 * Preserve the selection-removal, compaction and old-item insertion rules. */
#include "equipment_list_update.h"
#include "gba.h"
#include "structs/ewram.h"

struct MenuPage
{
    u8 pad_0[0xB];
    u8 subState;
    u8 pad_C[0x17 - 0xC];
    u8 item;
};

/**
 * @brief Remove the selected sole-owned item and current equipped item from the four-entry list, compact entries and reinsert the current equipped item under the existing rules.
 */
void EquipmentList_UpdateForEquip(struct MenuPage *page)
{
    struct PlayerStatus *stats;
    u8 count;
    u8 *list;
    u8 *owned;
    u8 cur;
    s32 item;
    s32 i;

    stats = &gEwramData->playerStatus;
    count = 0;
    list = NULL;
    owned = NULL;
    cur = 0xFF;
    item = page->item;
    switch (page->subState)
    {
    case 0:
        list = gEwramData->playerStatus.weaponList;
        owned = gEwramData->playerStatus.weaponInventory;
        cur = stats->equippedWeapon;
        break;
    case 1:
        list = gEwramData->playerStatus.armorList;
        owned = gEwramData->playerStatus.armorInventory;
        cur = gEwramData->playerStatus.equippedArmor;
        break;
    case 2:
        list = gEwramData->playerStatus.accessoryList;
        owned = gEwramData->playerStatus.armorInventory;
        cur = gEwramData->playerStatus.equippedAccessory;
        break;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == item && owned[item] == 1)
            list[i] = 0xFF;
        if (list[i] == cur)
            list[i] = 0xFF;
        if (list[i] == 0xFF)
            count++;
    }
    for (i = 2; i >= 0; i--)
    {
        while (list[i + 1] == 0xFF && list[i] != 0xFF)
        {
            list[i + 1] = list[i];
            list[i] = 0xFF;
            if (i <= 1)
                i++;
        }
    }
    if (count == 0)
    {
        for (i = 2; i >= 0; i--)
        {
            if (list[i] != 0xFF)
                list[i + 1] = list[i];
        }
        list[0] = cur;
    }
    else
    {
        list[count - 1] = cur;
    }
}
