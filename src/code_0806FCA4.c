/* Aim, release four effects, approach the sampled target and recover. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void KillerFish_SpawnOscillatingEffect(struct EwramData_EntityData *, s32, s32, s32, s32, s32);
static inline void CorrectSurface(struct EwramData_EntityData *entity, s32 flags)
{
    s32 classification = flags & 0x80;
    if (!classification)
    {
        entity->posY.part16.integer += Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
        entity->velY.whole = classification;
        entity->accelY.whole = 0x400;
    }
}
/**
 * @brief Sample a target point, wait, emit four effects, approach the sampled point and recover before returning to swimming.
 */
void KillerFish_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        {
            s32 targetX;
            u32 random;
            entity->flipX = Entity_IsTargetRight(entity);
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->timer = 0;
            targetX = Target_GetPixelX(entity);
            { s32 t_ = targetX + ({ register u32 bx_ asm("r2") = background->xPos.part16.integer; bx_; }) + (RandomNumberGenerator() & 7); entity->ext.killerFish.attackX = t_ - 4; } /* FAKEMATCH: pinned register */
            { s32 v_ = Target_GetPixelY(entity); { register u32 by_ asm("r6") = background->yPos.part16.integer; v_ += by_; } entity->ext.killerFish.attackY = v_ - 16; } /* FAKEMATCH: pinned register */
            entity->subState++;
        }
        break;
    case 1:
        if (entity->timer++ > 39)
        {
            entity->timer = 0;
            entity->subState++;
            Anim_StartIndexed(entity, 2, 0, 0);
        }
        break;
    case 2:
        if (entity->animStep)
        {
            s32 index;
            entity->timer = 0;
            entity->subState++;
            /* FAKEMATCH: r2 looks live over `index = 0`, so its reload takes r0 like the original */
            { register u32 z_ asm("r2"); asm("" : "=r"(z_)); index = 0; asm("" :: "r"(z_)); } /* FAKEMATCH: pinned register + empty asm */
            for (; index < 4; index++)
            {
                s32 x, y, vx, vy;
                u32 random;
                if (entity->flipX)
                {
                    x = entity->posX.whole - 0xA0000;
                    vx = -0x30000 - (RandomNumberGenerator() & 0xFFFF);
                }
                else
                {
                    x = entity->posX.whole + 0xA0000;
                    vx = (RandomNumberGenerator() & 0xFFFF) + 0x30000;
                }
                random = RandomNumberGenerator();
                { s32 t_ = x - 0x20000; y = 0x3FFFF; x = t_ + (random & y); }
                do { do { do { y = entity->posY.whole - (RandomNumberGenerator() & y); } while (0); } while (0); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
                if (index == 0)
                {
                    vy = -0x6000 - (RandomNumberGenerator() & 0x1FFF);
                    KillerFish_SpawnOscillatingEffect(entity, x, y, 15, vx, vy);
                }
                else
                {
                    vy = -0x4000 - (RandomNumberGenerator() & 0x1FFF);
                    KillerFish_SpawnOscillatingEffect(entity, x, y, 16, vx, vy);
                }
            }
        }
        break;
    case 3:
        if (Entity_AimVelocityAtPointByDistance(entity, (s16)entity->ext.killerFish.attackX << 16,
                         (s16)entity->ext.killerFish.attackY << 16, 0x20000))
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        if (entity->timer++ > 59)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        CorrectSurface(entity, Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x40000));
        break;
    case 4:
        EntityDampVelocityX(entity, 8);
        EntityDampVelocityY(entity, 8);
        CorrectSurface(entity, Entity_MoveAndCollide(entity, 0x80000, -0x70000, 1, 0x40000));
        if (entity->timer++ > 29)
        {
            entity->state = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        break;
    }
}
