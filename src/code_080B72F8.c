/* Dead Warrior charge, braking and death. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern u8 sDeadWarriorWalkProfile[];

/**
 * @brief Alive state (0): walk using the stored speed, accelerate, then
 * brake with particles and reverse direction when animation 2 finishes.
 * Retain the fallthrough from substate 1 into the acceleration logic.
 */
void DeadWarrior_UpdateCharge(struct EwramData_EntityData *entity)
{
    s32 timer;
    s32 x;
    s32 y;
    struct EwramData_EntityData *particle;
    Entity_Walk(entity, entity->ext.deadWarrior.speed, sDeadWarriorWalkProfile, 4);
    switch (entity->subState)
    {
    case 0:
        entity->ext.deadWarrior.speed = 0;
        entity->stage = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        if (Target_IsNearEntity(entity, 0x64, 0x3C) && *(u16 *)&entity->animStep == 0x100)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 2;
        }
        /* fall through */
    case 2:
        if ((u8)((u32)entity->timer % 25) == 0
            && Entity_IsInRect(entity, -0x40, -0x40, 0x170, 0x120)) PlaySong(0x7D);
        entity->timer++;
        if ((s32)entity->ext.deadWarrior.speed <= 0x1FFFF)
            entity->ext.deadWarrior.speed += 0x4000;
        entity->ext.deadWarrior.chargeFrame++;
        if (*(u16 *)&entity->animStep == 0x101) entity->stage++;
        if (entity->ext.deadWarrior.chargeFrame > 0x64)
        {
            PlaySong(0x127);
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 3;
            entity->timer = 0;
        }
        break;
    case 3:
        if ((s32)entity->ext.deadWarrior.speed > 0) entity->ext.deadWarrior.speed -= 0x1000;
        timer = entity->timer + 1;
        entity->timer = timer;
        if (!(timer & 3))
        {
            x = entity->posX.whole;
            y = entity->posY.whole;
            particle = Effect_SpawnShared(x, y, (RandomNumberGenerator() & 1) + 4, 6);
            if (particle != NULL)
            {
                particle->drawSkip = 1;
                particle->velX.whole = 0x8000;
                if (entity->flipX) particle->velX.whole = -particle->velX.whole;
            }
        }
        if (entity->animFlags & 4)
        {
            entity->ext.deadWarrior.chargeFrame = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 0;
            entity->flipX ^= 1;
        }
        break;
    }
}

/**
 * @brief Death state (1): launch backward and upward, flash and emit
 * particles. Return nonzero when the existing bounds or animation/timer
 * condition allows removal; preserve the timer comparison as written.
 */
s32 DeadWarrior_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x;
    s32 timer;
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 1;
        entity->velX.whole = -0x28000;
        if (entity->flipX) entity->velX.whole = -entity->velX.whole;
        entity->velY.whole = -0x28000;
        entity->accelY.whole = 0x2000;
        entity->timer = 0;
        /* fall through */
    case 1:
        if (entity->timer <= 0x27)
        {
            if (Entity_GetVisibilityPattern(0x28 - entity->timer)) entity->palette = 5;
            else entity->palette = entity->basePalette;
        }
        timer = entity->timer + 1;
        entity->timer = timer;
        if (!(timer & 7)) PlaySong(0x6E);
        if (!(entity->timer & 3))
        {
            x = entity->posX.whole + (RandomSigned(0x10) << 16);
            Effect_SpawnShared(x, entity->posY.whole + ((RandomSigned(0x10) - 0x20) << 16), 1, 7);
        }
        EntityAccelerateAndMove(entity, 0x80000);
        if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120)
            || ((entity->animFlags & 4) && entity->timer <= 0x80)) return 1;
        break;
    }
    return 0;
}
