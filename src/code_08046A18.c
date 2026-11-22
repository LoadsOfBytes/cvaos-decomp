#include "structs/ewram.h"

/**
 * @brief Mark every slot of the six four-entry equipment/soul lists as empty
 * (0xFF) using the existing |= 0xFF stores.
 */
void Player_ClearEquipmentAndSoulLists(void)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    s32 i;

    for (i = 0; i <= 3; i++)
    {
        player->weaponList[i] |= 0xFF;
        player->armorList[i] |= 0xFF;
        player->accessoryList[i] |= 0xFF;
        player->redSoulList[i] |= 0xFF;
        player->blueSoulList[i] |= 0xFF;
        player->yellowSoulList[i] |= 0xFF;
    }
}
