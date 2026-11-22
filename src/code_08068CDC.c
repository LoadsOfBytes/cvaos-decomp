/* Advance along an angle using a step bounded in its direction of change. */
#include "structs/ewram.h"
#include "code_08009A0.h"
/**
 * @brief Move the entity along angle by step = speed + time * acceleration,
 * clamped so it does not pass target, and return that step.
 */
s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time)
{
    s32 step = speed + time * acceleration;
    if ((target > speed && step > target) || (target < speed && step < target))
        step = target;
    entity->posX.whole += (step >> 8) * (Sin(angle + 0x4000) >> 8);
    entity->posY.whole += (step >> 8) * (Sin(angle) >> 8);
    return step;
}
