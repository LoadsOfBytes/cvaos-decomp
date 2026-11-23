/* Wall-crawling wheel: rolls around a block shedding debris, then homes in on its target while spinning. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_08013960.h"
#include "code_080D73B8.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);

#define SPIN entity->ext.raw.unk_4FC.part16.unk_4FC
#define SPIN_SPEED entity->ext.raw.unk_4FC.part16.unk_4FE[0]
#define SPEED entity->ext.raw.unk_504.whole

static inline void Debris(struct EwramData_EntityData *entity, s16 dx, s16 dy, s32 vy, s32 vx)
{
    struct EwramData_EntityData *p;

    if (!(entity->timer & 1))
    {
        p = Effect_CreateSharedFrame(entity->posX.whole + (dx << 16), entity->posY.whole + (dy << 16), (RandomNumberGenerator() & 1) + 0x40, 6);
        if (p != NULL)
        {
            p->velY.whole = RandomSigned(0x8000) + vy;
            p->accelY.whole = 0x2000;
            p->velX.whole = RandomSigned(0x20000) + vx;
        }
    }
}

/**
 * @brief Move the weapon around the room collision surfaces with debris,
 * then home toward the supplied owner and decelerate its rotation.
 * Return nonzero when the existing rotation-angle completion test passes.
 */
s32 DeathWeapon_UpdateWallTraversal(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    u16 *px = &entity->posX.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 x = *px + bg->xPos.part16.integer;
    u16 *py = &entity->posY.part16.integer;
    u16 y = *py + bg->yPos.part16.integer;

    switch (entity->stage)
    {
    case 0:
        SPEED = 0;
        SPIN_SPEED = 0;
        entity->stage = 1;
        entity->velY.whole = -0x18000;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        break;
    case 1:
        {
        s16 c = Collision_GetPushDown((s16)x, (s16)y - 0x18);
        if (c != 0)
        {
            Sound_PlayTracked(0x105);
            entity->posY.part16.integer = c + entity->posY.part16.integer;
            entity->velY.whole = 0;
            entity->stage = 2;
        }
        }
        break;
    case 2:
        Debris(entity, 0, -0x18, -0x8000, 0);
        entity->velX.whole = SPEED;
        {
        s32 c = Collision_GetPushLeft((s16)x + 0x18, (s16)y);
        if (c != 0)
        {
            {
                u32 t = (u16)entity->posX.part16.integer;
                entity->posX.part16.integer = c + t;
            }
            entity->velX.whole = 0;
            entity->stage = 3;
        }
        }
        break;
    case 3:
        Debris(entity, 0x18, 0, -0x8000, -0x20000);
        entity->velY.whole = SPEED;
        {
            s16 sx = x;
            s16 sy = y;
            s32 c2, c = Collision_GetPushLeft(sx + 8, sy + 0x28);
            if (c != 0)
                {
                    u32 t = (u16)entity->posX.part16.integer;
                    entity->posX.part16.integer = c + t;
                }
            c2 = Collision_GetPushUp(sx, sy + 0x18);
            if (c2 != 0)
            {
                {
                    u32 t = (u16)entity->posY.part16.integer;
                    entity->posY.part16.integer = c2 + t;
                }
                entity->velY.whole = 0;
                entity->stage = 4;
            }
        }
        break;
    case 4:
        Debris(entity, 0, 0x18, -0x28000, 0);
        entity->velX.whole = -SPEED;
        {
        s32 c = Collision_GetPushRight((s16)x - 0x20, (s16)y);
        if (c != 0)
        {
            {
                u32 t = (u16)entity->posX.part16.integer;
                entity->posX.part16.integer = c + t;
            }
            entity->velX.whole = 0;
            entity->stage = 5;
        }
        }
        break;
    case 5:
        Debris(entity, -0x18, 0, -0x8000, 0x20000);
        entity->velY.whole = -SPEED;
        {
        s16 c = Collision_GetPushUp((s16)x, (s16)y - 0x20);
        if (c != 0)
        {
            Sound_StopTracked(0x105);
            entity->posY.part16.integer = c + entity->posY.part16.integer;
            entity->stage = 6;
            SPIN_SPEED = 0xF877;
        }
        }
        break;
    case 6:
    {
        s32 tx, ty, ex, ey;
        u32 dist;
        s32 speed;

        SPIN += SPIN_SPEED;
        if (!(entity->timer & 0xF))
            PlaySong(0x135);
        tx = target->posX.whole;
        ty = target->posY.whole - 0x260000;
        ex = entity->posX.whole;
        ey = entity->posY.whole;
        dist = Sqrt(((ex - tx) >> 16) * ((ex - tx) >> 16) + ((ey - ty) >> 16) * ((ey - ty) >> 16)) << 16;
        Entity_AccelerateAlongAngle(entity,
                     ArcTan2((tx < 0 ? -(-tx >> 16) : tx >> 16) - (s16)entity->posX.part16.integer,
                             (ty < 0 ? -(-ty >> 16) : ty >> 16) - (s16)entity->posY.part16.integer),
                     speed = dist >> 4, 0, 0, 0);
        entity->velX.whole = entity->posX.whole - ex;
        entity->velY.whole = entity->posY.whole - ey;
        if (speed <= 0x7FFF)
            entity->stage = 7;
        break;
    }
    case 7:
        SPIN += SPIN_SPEED;
        if (!(entity->timer & 0x1F))
            PlaySong(0x135);
        if (SPIN_SPEED < -0x200)
        {
            SPIN_SPEED += 0x20;
            break;
        }
        if ((u16)((SPIN & 0x7FFF) - 0x3001) <= 0x7FE)
            return 1;
        break;
    }
    if (entity->stage <= 5)
    {
        EntityAccelerateAndMove(entity, 0x70000);
        SPIN += SPIN_SPEED;
    }
    if (entity->stage > 1)
    {
        entity->timer++;
        if (entity->stage <= 5 && SPEED <= 0x6FFFF)
        {
            SPEED += 0x4000;
            SPIN_SPEED = -0x100 - SPEED / 0x60;
        }
    }
    return 0;
}
