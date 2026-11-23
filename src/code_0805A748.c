/* Object36_FloorCorrection: landing depth of a point on top of a round object; matched. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "syscalls.h"
#include "structs/ewram.h"
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
static inline s32 ToPixels(s32 x) { return TO_PIXELS(x); }

/**
 * @brief Floor correction slot of the object 0x36 solid table: return the
 * push-up depth when the point is on the rotating surface, else 0.
 */
s16 Object36_FloorCorrection(struct EwramData_EntityData *entity, u16 tx, u16 ty)
{
    struct BgLayerState *cam;
    s32 ex;
    s32 ey;
    s32 sx;
    s32 sy;
    s32 dx;
    s32 dy;
    s32 r;
    s32 d;
    register s32 dist asm("r2"); /* FAKEMATCH: pinned register */

    cam = &gEwramData->bgInfo[1];
    ex = ToPixels(entity->posX.whole + cam->xPos.whole);
    {
        s32 t = entity->posY.whole + cam->yPos.whole;
        if (t < 0)
            ey = -(-t >> 16);
        else
            ey = t >> 16;
    }
    sy = (s16)ty;
    if (sy > ey - 4)
        return 0;
    sx = (s16)tx;
    {
        s32 x = ex - sx;
        dy = ey - sy;
        x *= x;
        dy *= dy;
        {
            s32 s = x + dy;
            dist = Sqrt(s);
        }
    }
    dx = sx - ex;
    dy = ey - sy;
    if (dx < 0)
        dx = -dx;
    r = (s16)OBJECT36_RADIUS(entity);
    if (dx > r - 6)
        return 0;
    if (dist < r - 8)
        return 0;
    if (dist >= r + 8)
        return 0;
    if (gEwramData->playerState.player->velY.whole < 0)
        return 0;
    /* the loop doubles the reference count of dy so that it gets r4 */
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    {
        s32 t = ex - sx;
        register s32 rr asm("r2") = r * r; /* FAKEMATCH: pinned register */
        t *= t;
        d = dy - (u16)Sqrt(rr - t);
    }
    while (0);
    if (d >= 0)
        return 0;
    return d;
}
