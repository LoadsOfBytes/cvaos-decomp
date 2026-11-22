/* Entity_AimAccelerationAtPoint: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "syscalls.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

static inline s32 AbsPx(s32 a, s32 b)
{
    s32 diff = a - b;

    if (diff < 0)
        return -((b - a) >> 16);
    return diff >> 16;
}

/**
 * @brief Aim acceleration fields accelX/accelY at a BG1-relative
 * room point through ArcTan2 and fixed sine products. Return 1
 * when the integer-pixel distance is within the supplied magnitude.
 */
s32 Entity_AimAccelerationAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
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
    entity->accelX.whole = outX;
    entity->accelY.whole = outY;
    if ((dx << 16) <= speed)
        return 1;
    return 0;
}
