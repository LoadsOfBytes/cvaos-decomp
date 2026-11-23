#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s16 Target_GetPixelX(struct EwramData_EntityData *entity);

static inline s32 Dist(struct EwramData_EntityData *entity)
{
    s32 d = Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
    if (d < 0)
        d = -d;
    return d;
}

/**
 * @brief 78880 | Hopping enemy AI: waits on the ground until the player is in
 * the 0xA0 box (or Enemy_IsDamaged), then (state 1) jumps at the player in
 * animation frame 3 – straight when Player_GetFocusedActionFlags reports a matching input,
 * otherwise a short hop (1/4) or a back jump, faster when far – and attacks
 * (state = 1) when the player is close; state 2 waits for the landing,
 * state 3 plays sound 0x85 during the attack animation.
 */
void Werejaguar_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    s32 res;
    s32 speed;
    s32 speed2;
    s32 accel;
    s32 h;
    s32 d;
    s32 v;

    switch (state)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = state;
        entity->accelX.whole = state;
        entity->velY.whole = 0x20000;
        entity->accelY.whole = 0x2800;
        d = Entity_MoveWithCollision(entity, 0x20000, -0x200000, 0x80000, 8);
        if (!(d & 4))
            break;
        if (Target_IsInRect(entity->posX.part16.integer - 0x50, entity->posY.part16.integer - 0x50, 0xA0, 0xA0) || Enemy_IsDamaged(entity))
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 6, 0, 1);
        }
        break;
    case 1:
        res = EntityIsOutsideScreen(entity, 4);
        if (res)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 0;
            break;
        }
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = res;
        entity->accelX.whole = res;
        speed = 0x20000;
        entity->velY.whole = speed;
        accel = 0x2800;
        entity->accelY.whole = accel;
        d = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(d & 4))
            break;
        if (entity->animStep != 3)
            break;
        h = entity->enemyId == 0x4B ? 0x28 : 0x78;
        if (Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - h, 0x78, h * 2))
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = state;
            entity->subState = res;
            entity->timer = res;
            entity->velX.whole = res;
            entity->accelX.whole = res;
            entity->velY.whole = speed;
            entity->accelY.whole = accel;
            break;
        }
        if ((Player_GetFocusedActionFlags() & 0x360) && Dist(entity) <= 0x4F)
        {
            if (entity->flipX)
                entity->velX.whole = -0x10000;
            else
                entity->velX.whole = 0x10000;
            entity->accelX.whole = 0;
            entity->velY.whole = -0x18000;
            entity->accelY.whole = 0x3000;
            entity->subState++;
            entity->animFlags |= 2;
            break;
        }
        else
        {
            if (!(RandomNumberGenerator() & 3) && Dist(entity) <= 0x4F)
            {
                if (RandomNumberGenerator() % 3 != 0)
                {
                    if (entity->flipX)
                        entity->velX.whole = -0x8000;
                    else
                        entity->velX.whole = 0x8000;
                }
                else
                {
                    Anim_StartIndexed(entity, 3, 0, 0);
                    entity->subState = 3;
                    break;
                }
            }
            else
            {
                if (entity->flipX)
                    entity->velX.whole = 0x10000;
                else
                    entity->velX.whole = -0x10000;
            }
            if (Dist(entity) > 0x63)
                entity->velX.whole += entity->velX.whole / 2;
        }
        entity->accelX.whole = 0;
        entity->velY.whole = -0x18000;
        entity->accelY.whole = 0x3000;
        entity->subState++;
        entity->animFlags |= 2;
        break;
    case 2:
        entity->animFlags |= 2;
        speed2 = 0x20000;
        d = Entity_MoveWithCollision(entity, speed2, -0x200000, 0x80000, 8);
        if (d & 4)
        {
            entity->animFlags &= ~2;
            entity->animTimer = 0xFA;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x2800;
            entity->subState = 1;
            Anim_StartIndexed(entity, 6, 0, 1);
        }
        break;
    case 3:
        if (!entity->animTimer && (entity->animStep == 1 || entity->animStep == 3))
            PlaySong(0x85);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->subState = 1;
        }
        break;
    }
}
