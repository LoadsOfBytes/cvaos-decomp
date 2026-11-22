/* Target-relative direction helpers, decompiled from original assembly. */
#include "structs/ewram.h"

/* 1 when the target is to the right, -1 to the left, 0 at the same x. */
s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 targetX;
    s32 selfX;

    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
        {
            targetX = entity->posX.whole;
            selfX = targetX;
            goto compare;
        }
    }
    else
        target = gEwramData->playerState.linkTargetProxy;
    targetX = target->posX.whole;
    selfX = entity->posX.whole;
compare:
    if (selfX < targetX)
        return 1;
    if (selfX > targetX)
        return -1;
    return 0;
}

/* Face the selected target, retaining the facing when their x is equal. */
void Entity_FaceTarget(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 targetX;
    s32 selfX;

    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
        {
            targetX = entity->posX.whole;
            selfX = targetX;
            goto compare;
        }
    }
    else
        target = gEwramData->playerState.linkTargetProxy;
    targetX = target->posX.whole;
    selfX = entity->posX.whole;
compare:
    if (targetX != selfX)
    {
        s32 facing = 0;
        if (selfX < targetX)
            facing = 1;
        entity->flipX = facing;
    }
}

