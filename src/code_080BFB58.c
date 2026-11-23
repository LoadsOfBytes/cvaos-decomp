/* Evil Butcher walking, throwing and death effects. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"

extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void EvilButcherProjectile_Update(struct EwramData_EntityData *);
extern void EvilButcherDebris_Update(struct EwramData_EntityData *);
extern void EvilButcherDeathParticle_Update(struct EwramData_EntityData *);
extern u8 sEvilButcherWalkProfile[];

/**
 * @brief Walk state (0): vary speed by animation, turn after cycles or
 * terrain obstruction, and enter throw state 1 when the target is near.
 */
void EvilButcher_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 odd;
    if (entity->subState != 0) return;
    if (*(u16 *)&entity->animStep == 0)
    {
        if (Target_IsNearEntity(entity, 0xA0, 0x78))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
            return;
        }
        if (entity->timer++ > 2)
        {
            entity->timer = 0;
            entity->flipX ^= 1;
        }
    }
    odd = entity->animStep;
    odd &= 1;
    speed = 0xA000 - (entity->animTimer << 11);
    if (odd) speed = 0x3000;
    if ((Entity_Walk(entity, speed, sEvilButcherWalkProfile, 0xC) & 0x1B) != 8
        && entity->timer <= 2)
        entity->timer = 3;
}

/**
 * @brief Throw state (1): face the target, spawn a projectile at the
 * combined animation-step/timer event 0x202 and return to walking
 * when the animation finishes.
 */
void EvilButcher_UpdateThrow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *projectile;
    u16 offset;
    s32 offsetX;
    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 0x202)
        {
            projectile = Entity_CreateObject(EvilButcherProjectile_Update);
            if (projectile != NULL)
            {
                offset = -0x10;
                projectile->posX.part16.integer = entity->posX.part16.integer;
                projectile->posY.part16.integer = entity->posY.part16.integer - 0x18;
                projectile->flipX = entity->flipX;
                if (projectile->flipX) offset = 0x10;
                offsetX = (s16)offset;
                offsetX += projectile->posX.part16.integer;
                projectile->posX.part16.integer = offsetX;
                projectile->attack = entity->attack;
                projectile->spawnParam1 = entity->spawnParam1;
            }
        }
        if (entity->animFlags & 4) entity->subState = 3;
        break;
    case 3:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}

/**
 * @brief Death state (2): spawn bouncing debris, display frame 6 and emit
 * random fragments or death particles for the countdown. Return nonzero
 * when removal is allowed.
 */
s32 EvilButcher_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *particle;
    s32 offset;
    s32 x;
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x1C1);
        particle = EntityCreateInRange(0x49, 0xCF, (u32 *)EvilButcherDebris_Update);
        if (particle != NULL)
        {
            offset = 0x120000;
            if (entity->flipX) offset = -offset;
            particle->posX.whole = entity->posX.whole + offset + RandomSigned(0x20000);
            particle->posY.whole = entity->posY.whole + RandomSigned(0x20000) - 0x200000;
            particle->scrollWithBg = 1;
            particle->spawnParam1 = entity->spawnParam1;
        }
        entity->animFrame = 6;
        entity->subState = 1;
        entity->timer = 0x40;
        return 0;
    case 1:
        if (!(entity->timer & 3))
        {
            if (RandomNumberGenerator() & 1)
            {
                x = entity->posX.whole + RandomSigned(0x80000);
                Effect_SpawnShared(x, entity->posY.whole + RandomSigned(0x100000) - 0x100000, 0xD, 8);
            }
            else
            {
                particle = EntityCreateInRange(0x49, 0xCF, (u32 *)EvilButcherDeathParticle_Update);
                if (particle != NULL)
                {
                    particle->posX.whole = entity->posX.whole + RandomSigned(0x80000);
                    particle->posY.whole = entity->posY.whole + RandomSigned(0x100000) - 0x100000;
                    particle->timer = (RandomNumberGenerator() & 7) + 8;
                    particle->scrollWithBg = 1;
                    particle->spawnParam1 = entity->spawnParam1;
                }
            }
        }
        if (entity->timer != 0)
        {
            if (entity->timer == 0x10) entity->drawSkip = 1;
            entity->timer--;
        }
        else return 1;
        break;
    }
    return 0;
}
