/* Julius_StartDeath. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code_080D73B8.h"

void Anim_StartIndexed(struct EwramData_EntityData *entity, s32 script, s32 mode, s32 flags);
void BlueSoul_StopAll(void);

/**
 * @brief Enter death state 16 from the Julius main update at nonpositive
 * HP: set scene flags, animation 0xC, stop blue souls and apply
 * the existing recoil velocity, player flags and sound.
 */
void Julius_StartDeath(struct EwramData_EntityData *entity)
{
    gEwramData->gameState.subMode = 2;
    gEwramData->gameState.subModeStep = 0;
    if (!(gEwramData->playerState.actionLockFlags & 0x800) && entity->animScriptIndex != 0xC)
    {
        Anim_StartIndexed(entity, 0xC, 1, 0);
        PLAYER_REQUESTED_ANIM(entity) = 0xFF;
        PLAYER_ACTION_FLAGS(entity) &= 0xFFDFFFFF;
    }
    BlueSoul_StopAll();
    PLAYER_ACTION_FLAGS(entity) &= 0xEFFFFE9F;
    gEwramData->playerState.actionLockFlags |= 0xE;
    gEwramData->playerState.pendingHitDamage = 0;
    if (!entity->flipX)
        entity->velX.whole = -0x10000;
    else
        entity->velX.whole = 0x10000;
    entity->velY.whole = -0x18000;
    entity->accelY.whole = 0x2000;
    entity->state = 0x10;
    {
        u8 two = 2;
        PLAYER_INVULNERABLE_TIMER(gEwramData->playerState.player) = two;
    }
    PlaySong(0x1E9);
}
