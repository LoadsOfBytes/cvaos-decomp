#include "code_08009A0.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);

static inline s32 Sq(s32 v) { return v * v; }

/**
 * @brief Unreferenced: compute the velocity for a ballistic arc from (x, y) to
 * the target with the given apex height, gravity and speed limit.
 */
s32 Entity_ComputeArcVelocity(struct EwramData_EntityData *entity, s32 x, s32 y, s32 targetX, s32 targetY, s32 height, s32 maxSpeed, u16 gravity, u16 time)
{
    s32 dist;
    s32 dx;
    s32 dy;
    u16 angle;
    u16 t;
    s16 s;
    s32 speed;
    s16 g;

    dist = Sqrt(Sq((x - targetX) >> 16) + Sq((y - targetY) >> 16)) << 16;
    angle = ArcTan2((targetX >> 16) - entity->posX.part16.integer, (targetY >> 16) - entity->posY.part16.integer);
    g = gravity;
    t = Div(height, g != 0 ? g : 1);
    dist += ((s16)t * (s16)t * g) >> 1;
    s = Sqrt(Div(dist, g != 0 ? g : 1));
    {
        s16 u = t = time + t;
        if (u > s)
            t = s * 2 - u - 1;
    }
    if ((s16)t < 0)
        speed = 0;
    else if ((g > 0 && (speed = time * g) > maxSpeed)
             || (g < 0 && (speed = time * g) < maxSpeed))
        speed = maxSpeed;
    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    return speed;
}
