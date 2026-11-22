/* Unreferenced helper: signed horizontal retreat speed based on player speed. */
#include "structs/ewram.h"
/**
 * @brief Carry slot of the solid table sBelmontWeaponSolidCallbacks that
 * Belmont_UpdatePoweredPhase registers for its weapon: signed horizontal
 * speed away from the player (at least 0x19000, from the player speed),
 * 0x10000 in animation 0.
 */
s32 BelmontWeaponSolid_GetCarryVelocity(struct EwramData_EntityData *entity)
{
    s32 speed;
    if(entity->animScriptIndex==0)
        speed=0x10000;
    else
    {
        s32 playerSpeed=gEwramData->playerState.player->velX.whole;
        if(playerSpeed<0)
            playerSpeed=-playerSpeed;
        speed=playerSpeed+0x1800;
        if(speed<0x19000)
            speed=0x19000;
    }
    if((s16)gEwramData->playerState.player->posX.part16.integer>(s16)entity->posX.part16.integer)
        speed=-speed;
    return speed;
}
