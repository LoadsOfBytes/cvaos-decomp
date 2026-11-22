#include "structs/ewram.h"

static inline s32 Item_GetTypeByGlobalId(s32 globalId)
{
    s32 type = 0;

    if (globalId == 0)
        return 0;
    globalId--;
    if (globalId <= 0x1F)
        type = 2;
    else if (globalId <= 0x5A)
        type = 3;
    else if (globalId <= 0x87)
        type = 4;
    return type;
}


static inline s32 Item_GetIndexByGlobalId(s32 globalId)
{
    s32 index = 0;

    if (globalId == 0)
        return 0;
    globalId--;
    if (globalId <= 0x1F)
        index = globalId;
    else if (globalId <= 0x5A)
        index = globalId - 0x20;
    else if (globalId <= 0x87)
        index = globalId - 0x5B;
    return index;
}


static inline u8 *ItemInventory_GetCountPtr(s32 type, s32 index)
{
    struct PlayerStatus *playerStatus = &gEwramData->playerStatus;

    switch (type)
    {
        case 2:
            return &playerStatus->itemInventory[index];
        case 3:
            return &playerStatus->weaponInventory[index];
        case 4:
            return &playerStatus->armorInventory[index];
    }
    return playerStatus->itemInventory;
}


/* Add one item to the inventory, capped at nine copies. */
/**
 * @brief Resolve the one-based item identifier and increment its inventory count below nine; preserve the invalid-type fallback.
 */
void ItemInventory_AddOneByGlobalId(s32 item)
{
    s32 type = Item_GetTypeByGlobalId(item);
    s32 index = Item_GetIndexByGlobalId(item);
    u8 *count = ItemInventory_GetCountPtr(type, index);
    if (*count < 9) (*count)++;
}
