/* Item and inventory helpers 0x08044F24..0x08044FFF (see code_08044E94.c).
 *
 * Global item ids are 1-based: 1..0x20 usable items (type 2), 0x21..0x5B
 * weapons (type 3), 0x5C..0x88 armor and accessories (type 4). */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

#define GOLD_MAX 999999


/**
 * @brief Resolve the global item identifier to type 2, 3 or 4; retain zero, out-of-range and signed comparison behavior.
 */
s32 Item_GetTypeByGlobalId(s32 globalId)
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

/**
 * @brief Resolve the global item identifier to its per-type index, retaining zero/default and signed range behavior.
 */
s32 Item_GetIndexByGlobalId(s32 globalId)
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

/**
 * @brief Add the supplied value through the unsigned gold field and cap results above 999999; preserve unsigned arithmetic behavior.
 */
void Player_AddGold(s32 amount)
{
    u32 *gold = &gEwramData->playerStatus.currentGold;

    *gold += amount;
    if (*gold > GOLD_MAX)
        *gold = GOLD_MAX;
}

/**
 * @brief Return the inventory count pointer for type 2..4 and index; unsupported types return the start of itemInventory.
 */
u8 *ItemInventory_GetCountPtr(s32 type, s32 index)
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

/**
 * @brief Advance only the animation of the child installed by SoulPickup_CreateContainer.
 */
void SoulPickup_UpdateContainerAnimation(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
}
