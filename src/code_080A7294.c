/* Tsuchinoko: jumping, ranged attack, and death states. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnParticleAnim13(s32, s32, s32, s32, s32, s32);
extern void Tsuchinoko_SpitShot(struct EwramData_EntityData *);
extern void Tsuchinoko_SpawnDroplet(struct EwramData_EntityData *);


/* The explicit final random-decision paths keep the original unsigned
 * mask/result in r1. Preserve RNG call order and short-circuit behavior.
 * These branches constrain agbcc allocation; a match does not establish
 * original source spelling or organization. */
/**
 * @brief Jump state machine (state 0): hop toward the target, drop debris
 * (Tsuchinoko_SpawnDroplet) and start the spit attack (state 1) at random.
 */
void Tsuchinoko_UpdateJump(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 90, entity->posY.part16.integer - 100, 180, 200) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 2)
        {
            if (Target_IsInRect(entity->posX.part16.integer - 60, entity->posY.part16.integer - 100, 120, 200))
                entity->flipX = !Entity_IsTargetRight(entity);
            else if (!Target_IsInRect(entity->posX.part16.integer - 100, entity->posY.part16.integer - 100, 200, 200))
                entity->flipX = Entity_IsTargetRight(entity);
            entity->subState++;
            entity->velY.whole = -0x18000;
            entity->accelY.whole = 0x2800;
            if (entity->flipX)
                entity->velX.whole = 0x10000;
            else
                entity->velX.whole = -0x10000;
        }
        break;
    case 2:
        {
            s32 flags = Entity_MoveWithCollision(entity, 0x10000, 0, 0x80000, 8);
            Entity_ClampXToRoom(entity, 1);
            if (flags & 4)
            {
                if ((RandomNumberGenerator() & 0x7F) < 16)
                {
                    entity->state = 1;
                    entity->subState = 0;
                }
                else
                {
                    if ((RandomNumberGenerator() & 0x7F) < 16 ||
                        ((s16)entity->ext.tsuchinoko.maxHp / 2 >= (s16)entity->hp && (RandomNumberGenerator() & 0x7F) < 32))
                        goto ranged;
                    if ((s16)entity->ext.tsuchinoko.maxHp / 4 >= (s16)entity->hp)
                    {
                        s32 random = RandomNumberGenerator();
                        u32 roll = 127;
                        roll &= random;
                        if (roll >= 64) goto jump;
                    }
                    else goto jump;
ranged:
                    entity->subState = 10;
                    goto done;
jump:
                    Anim_StartIndexed(entity, 1, 0, 0);
                    entity->subState = 1;
done:;
                }
            }
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->flipX = Entity_IsTargetRight(entity);
        entity->subState++;
        break;
    case 11:
        if (*(u16 *)&entity->animStep == 0x1403)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState++;
        }
        break;
    case 12:
        if (entity->animStep > 1)
        {
            if ((entity->timer++ & 15) == 0)
            {
                Tsuchinoko_SpawnDroplet(entity);
                PlaySong(0x152);
            }
        }
        if (entity->animDone)
            entity->deleteRequested = 1;
        break;
    }
}

