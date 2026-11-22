/* Apply the bounded step at an angle scaled by the time parameter. */
#include "structs/ewram.h"
#include "code_08009A0.h"
/**
 * @brief Add a bounded step along angle times time. The step is initial
 * speed plus acceleration times time, clamped toward its limit.
 * Preserve 16-bit direction truncation and products shifted by 8.
 */
void Entity_MoveWithTimedAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time)
{
    u16 direction = angle * time;
    s32 step = speed + acceleration * time;
    if ((target > speed && step > target) || (target < speed && step < target))
        step = target;
    entity->posX.whole += (step >> 8) * (Sin(direction + 0x4000) >> 8);
    entity->posY.whole += (step >> 8) * (Sin(direction) >> 8);
}

