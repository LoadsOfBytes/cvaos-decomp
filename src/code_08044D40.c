/* Bouncing object: collision with the room walls / floor. */
#include "gba.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "entity_hit.h"
#include "structs/ewram.h"

/* Wall / floor probes; declared with s32 returns (the caller truncates). */
extern s32 Collision_GetPushLeft(s32 x, s32 y);
extern s32 Collision_GetPushRight(s32 x, s32 y);
extern s32 Collision_GetPushUp(s32 x, s32 y);
extern s32 Collision_GetPushDown(s32 x, s32 y);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 amount);

/**
 * @brief Check collection, move with acceleration, damp horizontal speed and bounce against room walls/floor while stopping at the ceiling; retain signed probe conversions.
 */
void Pickup_UpdateBouncing(struct EwramData_EntityData *e)
{
    struct HitResult res;
    u16 *px;
    u16 x;
    s32 r;
    u16 y;
    struct BgLayerState *bg;

    Hit_CheckWithPlayer(&res, (u8 *)e);
    EntityAccelerateAndMove(e, 0x40000);
    px = &e->posX.part16.integer;
    bg = &gEwramData->bgInfo[1];
    x = *px + bg->xPos.part16.integer;
    y = e->posY.part16.integer + bg->yPos.part16.integer;
    EntityDampVelocityX(e, 0x40);
    if (e->velX.whole > 0)
    {
        r = Collision_GetPushLeft((s16)x, (s16)y - 4);
        x = r;
        if ((s16)r)
        {
            *px = ({ s32 t_ = (s16)x; t_; }) + *px;
            e->velX.whole = -e->velX.whole / 2;
        }
    }
    else if (e->velX.whole < 0)
    {
        r = Collision_GetPushRight((s16)x, (s16)y - 4);
        x = r;
        if ((s16)r)
        {
            *px = ({ s32 t_ = (s16)x; t_; }) + *px;
            e->velX.whole = -e->velX.whole / 2;
        }
    }
    {
        u16 *p = &e->posX.part16.integer;
        struct BgLayerState *bg2 = &gEwramData->bgInfo[1];
        x = *p + bg2->xPos.part16.integer;
    }
    if (e->velY.whole > 0)
    {
        r = Collision_GetPushUp((s16)x, (s16)y);
        x = r;
        if ((s16)r)
        {
            PlaySong(0x14F);
            e->posY.part16.integer += ({ s32 t_ = (s16)x; t_; });
            if (e->velY.whole <= 0x7FFF)
            {
                e->velY.whole = 0;
                e->accelY.whole = 0;
            }
            else
            {
                e->velY.whole = -e->velY.whole * 6 / 10;
            }
        }
    }
    else if (e->velY.whole < 0)
    {
        r = Collision_GetPushDown((s16)x, (s16)y - 4);
        x = r;
        if ((s16)r)
        {
            e->posY.part16.integer += ({ s32 t_ = (s16)x; t_; });
            e->velY.whole = 0;
        }
    }
    Anim_Update(e);
}
