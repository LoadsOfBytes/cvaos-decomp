/* Object helpers decompiled from assembly. */
#include "structs/ewram.h"
#include "code/code_080211F0.h"

extern u8 sObject0CSolidCallbacks[];

/* Copy the second word of the supplied data to velX when the player's
 * unk_4F4 flag 0x400000 is active. */
void Object0C_ApplyTriggerVelocity(struct EwramData_EntityData *entity, s32 *data)
{
    if (gEwramData->playerState.player->unk_4F4.whole & 0x400000)
        entity->velX.whole = data[1];
}

/**
 * @brief Trigger type 1: when the player has flag 0x400000 in unk_4F4, also set
 * flag 0x4000000.
 */
void Object0C_SetPlayerTriggerFlag(void)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    if (player->unk_4F4.whole & 0x400000)
        player->unk_4F4.whole |= 0x4000000;
}

/**
 * @brief Carry callback (slot 5) of the object 0x0C solid table sObject0CSolidCallbacks:
 * return its x velocity.
 */
s32 Object0C_GetCarryVelocity(struct EwramData_EntityData *entity)
{
    return entity->velX.whole;
}

void Object0CCreate(struct EwramData_EntityData *entity)
{
    Player_RegisterSolidObject(entity, sObject0CSolidCallbacks);
    entity->roomActive = 1;
}
