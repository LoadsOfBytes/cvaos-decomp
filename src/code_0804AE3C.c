/* Four-entry equipment lists: insert once into an empty slot, or slot 3. */
#include "structs/ewram.h"

extern const void *Item_GetInfo(s32, s32);

/**
 * @brief Select the equipment list from item type/record, avoid duplicates and insert into its first 0xFF slot or replace slot three when full.
 */
void EquipmentList_Insert(s32 type, s32 item)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list = NULL;
    const u8 *info = Item_GetInfo(type, item);
    s32 category = info[8];
    s32 i;
    if (type == 3)
        category = 0;
    else if (category == 0)
        category = 1;
    else
        category = 2;
    switch (category)
    {
        case 0:
            list = player->weaponList;
            break;
        case 1:
            list = player->armorList;
            break;
        case 2:
            list = player->accessoryList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == item)
            return;
    }
    for (i = 0; i <= 3; i++)
    {
        if (list[i] == 0xFF)
        {
            list[i] = item;
            break;
        }
    }
    if (i == 4)
        list[3] = item;
}

