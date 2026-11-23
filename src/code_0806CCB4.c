/* Entity target helpers 0x0806CCB4..0x0806CF2B (no upstream C existed).
 *
 * Most functions read a "target" entity: gEwramData->playerState.linkTargetProxy when
 * gameState.linkPlay is set, else playerState.enemyTarget when non-NULL, else the
 * entity itself (or 0 for velocities). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 EntityDistance(struct EwramData_EntityData *entity, struct EwramData_EntityData *target);

/**
 * @brief Repeat RNG modulo the first argument until the result is at
 * least the second. Preserve rejection sampling and the original
 * behavior when arguments cannot produce an accepted result.
 */
u32 RandomModuloAtLeast(u32 modulus, u32 minimum)
{
    u32 randomValue;

    do
    {
        randomValue = RandomNumberGenerator() % modulus;
    } while (randomValue < minimum);
    return randomValue;
}

/**
 * @brief 6CCD0 | Clear byte 0x504 and start sequence 0 via Anim_StartIndexed.
 */
void GiantGhost_ResetAnimation(struct EwramData_EntityData *entity)
{
    entity->ext.giantGhost.unusedFlag = 0;
    Anim_StartIndexed(entity, 0, 0, 0);
}

/**
 * @brief Draw normally, then outside bestiary preview draw another copy
 * 8 pixels right on layer 0x1D. Restore X, Y and sprite layer
 * after the second draw; no specific visual role is asserted.
 */
void Entity_DrawWithOffsetCopy(struct EwramData_EntityData *entity)
{
    s32 oldX;
    s32 oldY;
    u8 oldLayer;

    Sprite_DrawEntity(entity);
    if (!gEwramData->gameState.bestiaryPreview)
    {
        oldX = entity->posX.whole;
        oldY = entity->posY.whole;
        oldLayer = entity->spriteLayer;
        entity->posX.whole = oldX + 0x80000;
        entity->spriteLayer = 0x1D;
        Sprite_DrawEntity(entity);
        entity->posX.whole = oldX;
        entity->posY.whole = oldY;
        entity->spriteLayer = oldLayer;
    }
}

/**
 * @brief 6CD38 | Target x position (16.16).
 */
s32 Target_GetX(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->posX.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->posX.whole;
    return entity->posX.whole;
}

/**
 * @brief 6CD78 | Target y position (16.16).
 */
s32 Target_GetY(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->posY.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->posY.whole;
    return entity->posY.whole;
}

/**
 * @brief 6CDB8 | Target x position, integer part.
 */
s32 Target_GetPixelX(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;

    if (!gEwramData->gameState.linkPlay)
    {
        if (gEwramData->playerState.enemyTarget == NULL)
            target = entity;
        else
            target = gEwramData->playerState.enemyTarget;
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
    }
    return (s16)target->posX.part16.integer;
}

/**
 * @brief 6CDFC | Target y position, integer part.
 */
s32 Target_GetPixelY(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;

    if (!gEwramData->gameState.linkPlay)
    {
        if (gEwramData->playerState.enemyTarget == NULL)
            target = entity;
        else
            target = gEwramData->playerState.enemyTarget;
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
    }
    return target->posY.part16.integer;
}

/**
 * @brief Return alternate-target X velocity when flag linkPlay is set,
 * otherwise primary-target X velocity or 0 when no target exists.
 */
s32 Target_GetVelocityX(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->velX.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->velX.whole;
    return 0;
}

/**
 * @brief Return alternate-target Y velocity when flag linkPlay is set,
 * otherwise primary-target Y velocity or 0 when no target exists.
 */
s32 Target_GetVelocityY(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->velY.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->velY.whole;
    return 0;
}

/**
 * @brief Return the primary target horizontal flip, or the inverse
 * of the supplied entity flip if the primary target is NULL.
 * Unlike position/velocity helpers, ignore alternate-target mode.
 */
s32 Target_GetFlipXOrOpposite(struct EwramData_EntityData *entity)
{
    if (gEwramData->playerState.enemyTarget == NULL)
        return entity->flipX ^ 1;
    return gEwramData->playerState.enemyTarget->flipX;
}

/**
 * @brief Return 1 if the primary target exists and its unsigned
 * EntityDistance is at most the supplied limit, else 0.
 * This helper does not select the alternate target.
 */
s32 Target_IsWithinDistance(struct EwramData_EntityData *entity, u32 maxDistance)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;

    if (target == NULL || (u32)EntityDistance(entity, target) > maxDistance)
        return 0;
    return 1;
}
