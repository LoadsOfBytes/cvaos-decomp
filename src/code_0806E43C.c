/* Entity_StepTowardPoint. */
#include "structs/ewram.h"
#include "syscalls.h"

#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)

s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);

static inline s32 Square(s32 v)
{
    return v * v;
}

/* Inlined Entity_AngleToPoint (src/code_0802D1D8.c). */
static inline u16 AngleToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    return ArcTan2(INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer,
                   INT_TOWARD_ZERO(y) - entity->posY.part16.integer);
}

/**
 * @brief 6E43C | Step the entity toward the 16.16 point (x, y): speed is
 * the pixel distance in 16.16 shifted right by shift, moved along the angle
 * to the point with Entity_AccelerateAlongAngle; velX/velY get the actual displacement.
 *
 * @return the speed used
 */
u32 Entity_StepTowardPoint(struct EwramData_EntityData *entity, s32 x, s32 y, u8 shift)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    u32 dist = Sqrt(Square((oldX - x) >> 16) + Square((oldY - y) >> 16)) << 16;
    u16 angle = AngleToPoint(entity, x, y);
    u32 speed = dist >> (s8)shift;

    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - oldX;
    entity->velY.whole = entity->posY.whole - oldY;
    return speed;
}
