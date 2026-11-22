/* Signed pixel distance from the selected target. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include <string.h>

extern void RisingEffect_Update(struct EwramData_EntityData *);
extern const s8 sEntityVisibilityPattern[40];

s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity)
{
    u16 targetX;
    if (entity->enemyFlags & 4)
        targetX = gEwramData->playerState.player->posX.part16.integer;
    else
    {
        struct EwramData_EntityData *target;
        s32 x;
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == NULL)
            {
                x = (s16)entity->posX.part16.integer;
                goto convert;
            }
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
        x = (s16)target->posX.part16.integer;
    convert:
        targetX = x;
    }
    {
        s32 distance = entity->posX.part16.integer - (s16)targetX;
        distance <<= 16;
        return distance >> 16;
    }
}

/**
 * @brief Return signed 16-bit pixel distance entity Y minus selected target Y; missing default target gives zero.
 */
s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity)
{
    u16 targetY;
    if (entity->enemyFlags & 4)
        targetY = gEwramData->playerState.player->posY.part16.integer;
    else
    {
        struct EwramData_EntityData *target;
        s32 y;
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == NULL)
            {
                y = (s16)entity->posY.part16.integer;
                goto convert;
            }
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
        y = (s16)target->posY.part16.integer;
    convert:
        targetY = y;
    }
    {
        s32 distance = (u16)entity->posY.part16.integer - (s16)targetY;
        distance <<= 16;
        return distance >> 16;
    }
}

