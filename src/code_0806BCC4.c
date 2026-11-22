/* Check the entity facing against the selected target position.
 * The comparison works on a copy of selfX (x); the remaining pin keeps
 * selfX in r3 so the copy is not coalesced. */
#include "structs/ewram.h"

/* Whether the target lies in the direction the entity currently faces. */
/**
 * @brief Return 1 when the selected target lies strictly ahead of the
 * entity facing. Honor forced-player bit enemyFlags & 4, alternate
 * target mode and primary-target fallback; equal X returns 0.
 */
s32 Entity_IsFacingTarget(struct EwramData_EntityData *entity)
{
    s32 result = 0;
    struct EwramData_EntityData *target;
    s32 targetX;
    register s32 selfX asm("r3"); /* FAKEMATCH: pinned register */

    if (entity->enemyFlags & 4)
    {
        target = gEwramData->playerState.player;
        targetX = target->posX.whole;
        selfX = entity->posX.whole;
    }
    else if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
        {
            targetX = entity->posX.whole;
            selfX = targetX;
        }
        else
        {
            targetX = target->posX.whole;
            selfX = entity->posX.whole;
        }
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
        targetX = target->posX.whole;
        selfX = entity->posX.whole;
    }
    {
        s32 x = selfX;
        if (targetX != x && ((x < targetX && entity->flipX) || (selfX > targetX && !entity->flipX)))
            result = 1;
    }
    return result;
}
