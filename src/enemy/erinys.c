/*
 * Erinys (stubs), the Tsuchinoko create and jump state (the rest: tsuchinoko.c).
 * Built with -fno-cse-skip-blocks (Tsuchinoko_UpdateJump needs it; the rest compiles the same either way).
 */
#include "erinys.h"
#include "structs/ewram.h"
#include "core.h"
#include "graphics.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "tsuchinoko.h"
#include "valkyrie.h"

/* Forward this enemy variant to its shared implementation. */
void EnemyErinysUpdate(struct EwramData_EntityData *entity)
{
    EnemyValkyrieUpdate(entity);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyErinysCreate(struct EwramData_EntityData *entity)
{
    EnemyValkyrieCreate(entity);
}

extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Tsuchinoko_DamagePlayerOnContact(void);
extern void Tsuchinoko_OnHit(void);

void EnemyTsuchinokoCreate(struct EwramData_EntityData *entity)
{
    if (!Tsuchinoko_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Tsuchinoko_DamagePlayerOnContact, Tsuchinoko_OnHit);
    else
        Hit_Enable(entity, 8, Tsuchinoko_DamagePlayerOnContact, Tsuchinoko_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->ext.tsuchinoko.maxHp = entity->hp;
    {
        u32 random = RandomNumberGenerator();
        u32 mask = 0x7F;
        mask &= random;
        if (mask > 0x18)
        {
            entity->subState = 12;
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->animTimer = 0;
            entity->animStep = 8;
            Anim_Update(entity);
        }
    }
}

/* Tsuchinoko: jumping, ranged attack, and death states. */
extern s32 Target_IsInRect(s16, s16, s32, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Entity_ClampXToRoom(struct EwramData_EntityData *, s32);

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
