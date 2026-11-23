/* Object36_GetCarryVelocity: bounce vector of a round object against a point; matched. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "player_entity.h"
#include "syscalls.h"
#include "structs/ewram.h"
#define TO_PIXELS(d) ((d) < 0 ? -(-(d) >> 16) : (d) >> 16)
static inline s32 ToPixels(s32 x) { return TO_PIXELS(x); }

static inline u16 Distance(s32 x, s32 y, s32 tx, s32 ty)
{
    x -= tx;
    y -= ty;
    x *= x;
    y *= y;
    {
        register s32 s asm("r0") = x + y; /* FAKEMATCH: pinned register */
        return Sqrt(s);
    }
}

/**
 * @brief Carry callback slot of the object 0x36 solid table: return the carry
 * value for a point on the surface, else 0.
 */
s32 Object36_GetCarryVelocity(struct EwramData_EntityData *entity, u16 tx, u16 ty)
{
    struct BgLayerState *cam;
    s32 ex;
    s32 ey;
    s32 sx;
    s32 sy;
    s32 a;
    s32 b;
    s32 r;
    s32 second;
    s32 dy;

    cam = &gEwramData->bgInfo[1];
    {
        s32 d = entity->posX.whole + cam->xPos.whole;
        if (d < 0)
            ex = -(-d >> 16);
        else
            ex = d >> 16;
    }
    ey = ToPixels(entity->posY.whole + cam->yPos.whole);
    sy = (s16)ty;
    if (sy > ey - 4)
        return 0;
    sx = (s16)tx;
    {
        u16 dist = Distance(ex, ey, sx, sy);
        s32 radius = (s16)entity->ext.object36.radius + 6;
        dy = ey - sy;
        if (dist >= radius)
            return 0;
    }
    if (gEwramData->playerState.player->ext.player.actionFlags & 2)
        return 0;
    {
        register s32 dx asm("r1") = ex - sx; /* FAKEMATCH: pinned register */
        a = ArcTan2(dx, dy);
    }
    b = a - entity->ext.object36.spin;
    r = Sin(a + 0x4000);
    r *= (s16)entity->ext.object36.radius;
    {
        s32 second2 = Sin(b + 0x4000);
        {
            register s32 rad asm("r0") = (s16)entity->ext.object36.radius; /* FAKEMATCH: pinned register */
            second2 *= rad;
        }
        return r - second2;
    }
}
