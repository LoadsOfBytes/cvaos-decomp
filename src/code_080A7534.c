#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Tsuchinoko_SpitShot(struct EwramData_EntityData *);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnParticleAnim13(s32, s32, s32, s32, s32, s32);

/* Attack on animation event 3, then resume jumping when it finishes. */
void Tsuchinoko_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->flipX = Entity_IsTargetRight(entity);
        entity->subState++;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 3)
        {
            Tsuchinoko_SpitShot(entity);
            PlaySong(0x85);
        }
        if (entity->animDone)
        {
            entity->state = 0;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    }
}
/* Fall and blink for 60 frames; emit effects at frames 0 and 8. */
void Tsuchinoko_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x, y;
    switch (entity->subState)
    {
    case 0:
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0x1000;
        {
            x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            {
                s32 vx = RandomNumberGenerator();
                s32 vy;
                u32 velMask;
                s32 velOffset;
                velMask = 0x1FFFF;
                vx &= velMask;
                velOffset = -0x10000;
                vx += velOffset;
                vy = RandomNumberGenerator();
                vy &= velMask;
                vy += velOffset;
                Effect_SpawnParticleAnim13(x, y, vx, vy, entity->spriteLayer - 1, 2);
            }
        }
        entity->timer = 60;
        entity->subState++;
        /* fall through */
    case 1:
        if (entity->timer == 52)
        {
            x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            {
                s32 vx = RandomNumberGenerator();
                s32 vy;
                u32 velMask;
                s32 velOffset;
                velMask = 0x1FFFF;
                vx &= velMask;
                velOffset = -0x10000;
                vx += velOffset;
                vy = RandomNumberGenerator();
                vy &= velMask;
                vy += velOffset;
                Effect_SpawnParticleAnim13(x, y, vx, vy, entity->spriteLayer - 1, 2);
            }
        }
        EntityAccelerateAndMove(entity, 0x8000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        break;
    }
}
