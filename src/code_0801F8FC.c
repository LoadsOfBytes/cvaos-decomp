/* Julius_UpdateJumpControl. The flags and
 * velocity are re-read from the entity each time (the CSE copies match only
 * that way). */
#include "code_08009A0.h"
#include "player_entity.h"

void Julius_UpdateVerticalMovement(struct EwramData_EntityData *entity);

/**
 * @brief Unless actionLockFlags bit 0x10 is set, cut rising speed when jump is
 * released, add held-jump lift near the apex and slow falling under
 * flag 0x400000, then update Julius vertical movement.
 */
void Julius_UpdateJumpControl(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;

    if (ew->playerState.actionLockFlags & 0x10)
        return;
    if ((PLAYER_ACTION_FLAGS(entity) & 0x1E) && !(PLAYER_ACTION_FLAGS(entity) & 0x18))
    {
        if (entity->velY.whole < -0x4000 && !(ew->inputData.playerHeldInput & ew->playerStatus.jumpButton))
        {
            entity->velY.whole = -0x4000;
            entity->accelY.whole = -0x2000;
        }
        if (entity->velY.whole > -0x20000 && entity->velY.whole < 0x2000 && (gEwramData->inputData.playerHeldInput & gEwramData->playerStatus.jumpButton))
        {
            entity->accelY.whole -= 0x800;
            if (entity->accelY.whole < -0x2000)
                entity->accelY.whole = -0x2000;
        }
        if ((PLAYER_ACTION_FLAGS(entity) & 0x400000) && entity->velY.whole > 0x1800)
            entity->velY.whole -= 0x1800;
    }
    Julius_UpdateVerticalMovement(entity);
}
