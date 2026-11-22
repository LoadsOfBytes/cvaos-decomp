/* Helpers 0x0806E3B0..0x0806E43B. */
#include "syscalls.h"
#include "structs/ewram.h"

/**
 * @brief Return alternate-target horizontal flip when linkPlay is
 * set, else primary-target flip or the supplied entity flip.
 * Unlike Target_GetFlipXOrOpposite, this fallback is not inverted.
 */
u32 Target_GetFlipX(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;

    if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
            target = entity;
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
    }
    return target->flipX;
}

/* Signed integer-pixel difference param_0 - param_1 (16.16 inputs). */
static inline s32 AbsPx(s32 a, s32 b)
{
    s32 diff = a - b;

    if (diff < 0)
        return -((b - a) >> 16);
    return diff >> 16;
}

/**
 * @brief Return integer-pixel Euclidean distance to the supplied local
 * fixed-point position as a fixed-point result. Preserve signed
 * pixel truncation before squaring and the Sqrt result shift.
 */
s32 Entity_DistanceToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s32 dx2 = AbsPx(x, entity->posX.whole);
    s32 dy;

    dx2 = dx2 * dx2;
    dy = AbsPx(y, entity->posY.whole);
    return Sqrt(dx2 + dy * dy) << 16;
}
