/* Entity_AimVelocityAtPoint (steer toward a point). */
#include "syscalls.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Signed integer-pixel difference param_0 - param_1 (16.16 inputs). */
static inline s32 AbsPx(s32 a, s32 b)
{
    s32 diff = a - b;

    if (diff < 0)
        return -((b - a) >> 16);
    return diff >> 16;
}

/**
 * @brief 694B8 | Aim the velocity (speed speed) from the entity (screen
 * position + camera of background 1) at the world point (targetX,
 * targetY); 1 when the point is at most speed away.
 */
s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx;
    s32 dy;
    s32 outX;
    s32 outY;
    u16 angle;

    dx = AbsPx(targetX, bg->xPos.whole + entity->posX.whole);
    dy = AbsPx(targetY, bg->yPos.whole + entity->posY.whole);
    angle = ArcTan2(dx, dy);
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    outX = FixedMultiply(dx, speed);
    outY = FixedMultiply(dy, speed);
    dx = AbsPx(targetX, bg->xPos.whole + entity->posX.whole);
    dx = dx * dx;
    dy = AbsPx(targetY, bg->yPos.whole + entity->posY.whole);
    dy = dy * dy;
    dx = Sqrt(dx + dy);
    entity->velX.whole = outX;
    entity->velY.whole = outY;
    if ((dx << 16) <= speed)
        return 1;
    return 0;
}
