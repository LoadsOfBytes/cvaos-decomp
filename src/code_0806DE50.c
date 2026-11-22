#include "structs/ewram.h"

/* Horizontal distance from the selected target, zero without a target. */
/**
 * @brief Return entity X minus selected target X in fixed-point units.
 * Honor forced-player bit enemyFlags & 4 and alternate-target mode;
 * return 0 when the normal primary target is NULL.
 */
s32 Entity_GetTargetOffsetX(struct EwramData_EntityData *entity)
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
                x = entity->posX.whole;
            else
                x = target->posX.whole;
            current = entity->posX.whole;
            return current - x;
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
    }
    x = target->posX.whole;
    return entity->posX.whole - x;
}
