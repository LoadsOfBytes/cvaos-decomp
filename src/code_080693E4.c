/* Set velocity towards a world point, accounting for background scroll. */
#include "structs/ewram.h"
#include "syscalls.h"
#define PIXEL_DIFF(a, b) ((a) - (b) < 0 ? -(((b) - (a)) >> 16) : ((a) - (b)) >> 16)
/**
 * @brief Set velocity toward a BG1-relative room point by dividing
 * speed times signed pixel differences by the 16-bit distance.
 * Return 1 within one speed step; retain zero-distance division.
 */
s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 dx = PIXEL_DIFF(targetX, background->xPos.whole + entity->posX.whole);
    s32 dy;
    u16 distance;
    s32 vx, vy;
    dx *= dx;
    dy = PIXEL_DIFF(targetY, background->yPos.whole + entity->posY.whole);
    dy *= dy;
    distance = Sqrt(dx + dy);
    vx = speed * PIXEL_DIFF(targetX, background->xPos.whole + entity->posX.whole) / distance;
    vy = speed * PIXEL_DIFF(targetY, background->yPos.whole + entity->posY.whole) / distance;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    if ((distance << 16) <= speed) return 1;
    return 0;
}


