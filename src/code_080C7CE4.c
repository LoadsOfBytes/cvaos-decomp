/* Killer Doll movement and attack state machine, reconstructed from the
 * original assembly. */
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
extern void KillerDoll_ResolveMovementBounds(struct EwramData_EntityData *entity);
/* This caller tests the full return register (no u8 truncation). */
extern s32 Collision_GetTileOnScreen(s32 x, s32 y);

#define TIMER_X(e) ((e)->unk_4F8.part16.unk_4F8)
#define TIMER_Y(e) ((e)->unk_4F8.part16.unk_4FA)
#define TIMER_Z(e) ((e)->unk_4FC.part16.unk_4FC)
#define ANIM(e, n, loop) do { if ((e)->animScriptIndex != (n)) Anim_StartIndexed((e), (n), 0, (loop)); } while (0)
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Dispatch awakening, floating movement, turning, charge and floor-wait substates; integrate velocity and use three countdowns to select behavior.
 */
void KillerDoll_UpdateActive(struct EwramData_EntityData *entity)
{
    Target_GetX(entity);
    Target_GetY(entity);
    if ((s16)TIMER_X(entity) != 0)
        TIMER_X(entity)--;
    if (TIMER_Y(entity) != 0)
        TIMER_Y(entity)--;
    if (TIMER_Z(entity) != 0)
        TIMER_Z(entity)--;

    switch (entity->subState)
    {
        case 0:
            ANIM(entity, 0, 1);
            if (Entity_IsTargetWithinRange(entity, 0x3C0000) &&
                (Target_GetX(entity) != entity->posX.whole ||
                 Target_GetY(entity) != entity->posY.whole))
            {
                ANIM(entity, 1, 0);
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 1:
            if (entity->animDone)
            {
                ANIM(entity, 2, 0);
                entity->velY.whole = -0x4000;
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 2:
            if (entity->timer++ > 0x20)
            {
                ANIM(entity, 4, 1);
                entity->velY.whole = 0;
                entity->timer = 0;
                entity->subState = 4;
                entity->spriteLayer = 3;
            }
            break;
        case 3:
            switch (entity->stage)
            {
                case 0:
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    if ((RandomNumberGenerator() & 3) == 0)
                    {
                        Entity_FaceTarget(entity);
                        entity->flipX = 1 - entity->flipX;
                        entity->velX.whole = 0x4000;
                        if ((s16)Entity_GetTargetDistanceX(entity) < 0)
                            entity->velX.whole = -entity->velX.whole;
                    }
                    else
                    {
                        entity->velX.whole = 0x8800;
                        entity->velY.whole = 0x8800;
                        if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                            entity->velX.whole = -entity->velX.whole;
                        if ((s16)Entity_GetTargetDistanceY(entity) > 0)
                            entity->velY.whole = -entity->velY.whole;
                    }
                    ANIM(entity, 4, 1);
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
                    KillerDoll_ResolveMovementBounds(entity);
                    break;
            }
            break;
        case 4:
            switch (entity->stage)
            {
                case 0:
                    entity->accelX.whole = 0;
                    entity->velX.whole = 0;
                    entity->accelY.whole = 0;
                    entity->velY.whole = 0;
                    ANIM(entity, 3, 0);
                    if (entity->animDone)
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                        TIMER_X(entity) += 0x20;
                        TIMER_Z(entity) += 0x20;
                        TIMER_Y(entity) = RandomNumberGenerator() % 0x100 + 0x20;
                        Entity_FaceTarget(entity);
                    }
                    break;
            }
            break;
        case 5:
            switch (entity->stage)
            {
                case 0:
                {
                    u16 angle;
                    ANIM(entity, 5, 0);
                    angle = Entity_GetTargetAngleAbove16(entity);
                    entity->velX.whole = FixedMultiply(Sin(angle + 0x14000), 0x10000);
                    entity->velY.whole = FixedMultiply(Sin(angle + 0x10000), 0x10000);
                    entity->timer = ABS((Target_GetX(entity) - entity->posX.whole) / entity->velX.whole);
                    entity->stage++;
                }
                case 1:
                {
                    if ((s8)entity->timer-- == 0)
                        entity->stage++;
                    break;
                }
                case 2:
                    if (!entity->flipX || (s16)Entity_GetTargetDistanceX(entity) >= 0)
                    {
                        entity->subState = 4;
                        entity->stage = 0;
                    }
                    else
                    {
                        entity->subState = 3;
                        entity->stage = 0;
                    }
                    TIMER_X(entity) = RandomNumberGenerator() % 0x800 + 0x80;
                    TIMER_Y(entity) += 0x20;
                    TIMER_Z(entity) += 0x20;
                    break;
            }
            break;
        case 6:
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
                        ANIM(entity, 6, 0);
                        entity->timer = RandomNumberGenerator() % 0x40 + 0x40;
                        entity->stage++;
                    }
                    break;
                case 2:
                    if (--entity->timer == 0xFF)
                    {
                        TIMER_X(entity) += 0x20;
                        TIMER_Y(entity) += 0x20;
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
    if (entity->subState == 3)
    {
        if (TIMER_Z(entity) == 0)
        {
            entity->subState = 6;
            entity->stage = 0;
        }
        if (TIMER_Y(entity) == 0)
        {
            entity->subState = 4;
            entity->stage = 0;
        }
        if ((s16)TIMER_X(entity) == 0)
        {
            if ((entity->flipX && (s16)Entity_GetTargetDistanceX(entity) < 0) ||
                (!entity->flipX && (s16)Entity_GetTargetDistanceX(entity) > 0))
            {
                if (Entity_IsTargetWithinRange(entity, 0x280000))
                {
                    entity->subState = 5;
                    entity->stage = 0;
                }
            }
        }
    }
}
