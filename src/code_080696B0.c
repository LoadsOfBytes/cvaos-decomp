/* Aim velocity at a point and immediately integrate the movement. */
#include "structs/ewram.h"
#include "syscalls.h"
#define PIXEL_DIFF(a, b) ((a) - (b) < 0 ? -(((b) - (a)) >> 16) : ((a) - (b)) >> 16)
/**
 * @brief Set the velocity to speed (16.16) along the line to (targetX, targetY) and
 * move one step; return 1 when the remaining distance is within speed, else 0.
 */
s32 Entity_MoveTowardPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{

    s32 dx = PIXEL_DIFF(targetX, entity->posX.whole);
    s32 dy;
    u16 distance;
    s32 vx, vy;
    dx *= dx;
    dy = PIXEL_DIFF(targetY, entity->posY.whole);
    dy *= dy;
    distance = Sqrt(dx + dy);
    vx = speed * PIXEL_DIFF(targetX, entity->posX.whole) / distance;
    vy = speed * PIXEL_DIFF(targetY, entity->posY.whole) / distance;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->posX.whole += vx;
    entity->posY.whole += vy;
    if ((distance << 16) <= speed) return 1;
    return 0;
}



