/* Entity_MoveXWithClampedStep / Entity_MoveYWithClampedStep (clamped steps). */
#include "structs/ewram.h"

/**
 * @brief Add initial step plus time times acceleration to X, clamped
 * against the supplied step limit in the direction from initial
 * to limit. Return the applied step; this does not set velocity.
 */
s32 Entity_MoveXWithClampedStep(struct EwramData_EntityData *entity, s32 speed, s32 targetSpeed, s16 accel, u16 steps)
{
    s32 step = speed + steps * accel;

    if ((targetSpeed > speed && step > targetSpeed) || (targetSpeed < speed && step < targetSpeed))
        step = targetSpeed;
    entity->posX.whole += step;
    return step;
}

/**
 * @brief Add initial step plus time times acceleration to Y, clamped
 * against the supplied step limit in the direction from initial
 * to limit. Return the applied step; this does not set velocity.
 */
s32 Entity_MoveYWithClampedStep(struct EwramData_EntityData *entity, s32 speed, s32 targetSpeed, s16 accel, u16 steps)
{
    s32 step = speed + steps * accel;

    if ((targetSpeed > speed && step > targetSpeed) || (targetSpeed < speed && step < targetSpeed))
        step = targetSpeed;
    entity->posY.whole += step;
    return step;
}
