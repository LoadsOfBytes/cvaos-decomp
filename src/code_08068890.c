/* Move a fraction of the remaining distance towards another entity. */
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "syscalls.h"
static inline s32 MoveAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time)
{
    s32 step = speed + time * acceleration;
    if ((target > speed && step > target) || (target < speed && step < target))
        step = target;
    entity->posX.whole += (step >> 8) * (Sin(angle + 0x4000) >> 8);
    entity->posY.whole += (step >> 8) * (Sin(angle) >> 8);
    return step;
}

/**
 * @brief Move toward the supplied entity using distance shifted by
 * the signed interpretation of shift. Return that fixed-point step.
 * Preserve integer-pixel distance, angle conversion and helper call.
 */
s32 Entity_MoveTowardEntityFraction(struct EwramData_EntityData *entity, struct EwramData_EntityData *target, u8 shift)
{
    s32 dx = (entity->posX.whole - target->posX.whole) >> 16;
    s32 dy;
    u32 distance;
    u16 angle;

    dx *= dx;
    dy = (entity->posY.whole - target->posY.whole) >> 16;
    dy *= dy;
    distance = Sqrt(dx + dy) << 16;
    angle = ArcTan2((s16)(target->posX.part16.integer - entity->posX.part16.integer), (s16)(target->posY.part16.integer - entity->posY.part16.integer));
    distance >>= (s8)shift;
    MoveAngle(entity, angle, distance, 0, 0, 0);
    return distance;
}


