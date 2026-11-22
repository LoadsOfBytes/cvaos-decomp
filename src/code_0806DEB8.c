#include "structs/ewram.h"

/* Vertical distance from the selected target, zero without a target. */
/**
 * @brief Return entity Y minus selected target Y in fixed-point units.
 * Honor forced-player bit enemyFlags & 4 and alternate-target mode;
 * return 0 when the normal primary target is NULL.
 */
s32 Entity_GetTargetOffsetY(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 x, current;
    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else
    {
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == 0)
                x = entity->posY.whole;
            else
                x = target->posY.whole;
            current = entity->posY.whole;
            return current - x;
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
    }
    x = target->posY.whole;
    return entity->posY.whole - x;
}

