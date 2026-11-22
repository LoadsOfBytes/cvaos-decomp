/* Soma_UpdateJumpControl; unk_4F4 is
 * read from the entity each time (as in Julius_UpdateJumpControl). */
#include "code_08009A0.h"
#include "player_entity.h"
#include "structs/ewram.h"

void Soma_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/**
 * @brief Apply held/released-jump vertical control and existing ability
 * flag adjustments, then call Soma vertical movement. Return early
 * under player flag 0x10; preserve the asymmetric speed thresholds.
 */
void Soma_UpdateJumpControl(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;

    if (ew->playerState.actionLockFlags & 0x10)
        return;
    if (PLAYER_ACTION_FLAGS(entity) & 0x1E)
    {
        if (!(PLAYER_ACTION_FLAGS(entity) & 0x18))
        {
            if (entity->velY.whole < -0x4000 && !(ew->inputData.playerHeldInput & ew->playerStatus.jumpButton))
            {
                entity->velY.whole = -0x4000;
                entity->accelY.whole = -0x2000;
            }
            if (entity->velY.whole >= -0x1BFFF && entity->velY.whole <= 0x1FFF
                && (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.jumpButton))
            {
                entity->accelY.whole -= 0x800;
                if (entity->accelY.whole < -0x2000)
                    entity->accelY.whole = -0x2000;
            }
        }
        if (((gEwramData->playerStatus.soulStateFlags & 0x100) || (PLAYER_ACTION_FLAGS(entity) & 0x400000)) && entity->velY.whole > 0x2800)
            entity->velY.whole -= 0x2800;
        if (gEwramData->playerStatus.soulStateFlags & 0x800)
        {
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
        }
    }
    Soma_UpdateVerticalMovement(entity);
}
