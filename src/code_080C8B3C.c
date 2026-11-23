/* Lightning Doll movement and electrical attack states. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, s32 range);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern u16 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *entity);
extern void LightningDoll_ResolveMovementBounds(struct EwramData_EntityData *entity);
extern s32 Collision_GetTileOnScreen(s32 x, s32 y);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity, s32 width, s32 height);
extern void Sound_PlayTracked(s32 id);
extern void Sound_StopTracked(s32 id);

#define TIMER_X(e) (e->ext.lightningDoll.attackTimer)
#define TIMER_Y(e) (e->ext.lightningDoll.moveTimer)
#define TIMER_Z(e) (e->ext.lightningDoll.restTimer)
#define ANIM(e, n, loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e), (n), 0, (loop)); } while (0)
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Dispatch awakening, randomized flight, electrical attack and floor-wait substates; integrate velocity and select behavior from countdowns and target checks.
 */
void LightningDoll_UpdateActive(struct EwramData_EntityData *entity)
{
    if ((s16)TIMER_X(entity) != 0)
        TIMER_X(entity)--;
    if (TIMER_Z(entity) != 0)
        TIMER_Z(entity)--;
    switch (entity->subState)
    {
        case 0:
            ANIM(entity, 0, 1);
            if (Target_IsNearEntity(entity, 240, 128) &&
                (Target_GetX(entity) != entity->posX.whole || Target_GetY(entity) != entity->posY.whole))
            {
                ANIM(entity, 1, 0);
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        case 1:
            if (entity->animDone)
            {
                entity->timer = 0;
                entity->subState = 2;
                entity->stage = 0;
                entity->spriteLayer = 3;
            }
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    Entity_FaceTarget(entity);
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    switch (RandomNumberGenerator() % 6)
                    {
                        case 0:
                        case 1:
                        case 2:
                            entity->velX.whole = 0x8000;
                            entity->velY.whole = 0x8000;
                            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                                entity->velX.whole = -entity->velX.whole;
                            if ((s16)Entity_GetTargetDistanceY(entity) > 0)
                                entity->velY.whole = -entity->velY.whole;
                            TIMER_Y(entity) = RandomNumberGenerator() % 0x100 + 0x80;
                            break;
                        case 3:
                            entity->flipX = 1 - entity->flipX;
                            entity->velX.whole = 0x4000;
                            if ((s16)Entity_GetTargetDistanceX(entity) < 0)
                                entity->velX.whole = -entity->velX.whole;
                            TIMER_Y(entity) = RandomNumberGenerator() % 0x100 + 0x80;
                            break;
                        case 4:
                        case 5:
                        {
                            u16 angle = Entity_GetTargetAngleAbove16(entity);
                            entity->velX.whole = FixedMultiply(Sin(angle + 0x14000), 0x10000);
                            entity->velY.whole = FixedMultiply(Sin(angle + 0x10000), 0x10000);
                            TIMER_Y(entity) = ABS((Target_GetX(entity) - entity->posX.whole) / entity->velX.whole);
                            break;
                        }
                    }
                    ANIM(entity, 2, 1);
                    entity->stage++;
                    break;
                case 1:
                    if (!entity->flipX)
                    {
                        if (entity->velX.whole > 0)
                            entity->flipX = 1;
                    }
                    if (entity->flipX)
                    {
                        if (entity->velX.whole < 0)
                            entity->flipX = 0;
                    }
                    if (TIMER_Y(entity)-- <= 0)
                        entity->stage = 0;
                    LightningDoll_ResolveMovementBounds(entity);
                    break;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    ANIM(entity, 3, 0);
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    entity->timer = RandomNumberGenerator() % 0x100 + 0x80;
                    entity->stage++;
                    Sound_PlayTracked(0x75);
                case 1:
                    if (entity->animDone)
                    {
                        ANIM(entity, 4, 1);
                        entity->stage++;
                    }
                    break;
                case 2:
                    if ((entity->timer & 15) == 0 && entity->timer > 32)
                        Sound_PlayTracked(0x75);
                    if ((s8)entity->timer-- == 0)
                        entity->stage++;
                    break;
                case 3:
                    Sound_StopTracked(0x75);
                    entity->subState = 2;
                    entity->stage = 0;
                    TIMER_X(entity) = RandomNumberGenerator() % 0x20 + 0x10;
                    TIMER_Z(entity) += 0x20;
                    break;
            }
            break;
        case 4:
            switch (entity->stage)
            {
                case 0:
                    entity->spriteLayer = 9;
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    entity->stage = 0;
                case 1:
                    if (Collision_GetTileOnScreen((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer) == 0)
                        entity->posY.part16.integer++;
                    else
                    {
                        ANIM(entity, 5, 0);
                        entity->timer = RandomNumberGenerator() % 0x40 + 0x40;
                        entity->stage++;
                    }
                    break;
                case 2:
                    if (--entity->timer == 0xFF)
                    {
                        TIMER_Z(entity) = RandomNumberGenerator() % 0x400 + 0x200;
                        ANIM(entity, 1, 0);
                        entity->subState = 1;
                        entity->stage = 0;
                    }
                    break;
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    if (entity->subState == 2)
    {
        if (TIMER_Z(entity) == 0)
        {
            entity->subState = 4;
            entity->stage = 0;
        }
        if ((s16)TIMER_X(entity) == 0)
        {
            if ((entity->flipX && (s16)Entity_GetTargetDistanceX(entity) < 0) ||
                (!entity->flipX && (s16)Entity_GetTargetDistanceX(entity) > 0))
            {
                u32 chance;
                if (Entity_IsTargetWithinRange(entity, 0x140000))
                    chance = 0;
                else
                    chance = RandomNumberGenerator() & 0x3FF;
                if (chance == 0)
                {
                    entity->subState = 3;
                    entity->stage = chance;
                }
            }
        }
    }
}
