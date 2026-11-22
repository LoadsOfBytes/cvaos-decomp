/* Bat state machine, reconstructed from original ROM assembly. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Bat_IsTargetBelow(struct EwramData_EntityData *, u16, u16);
extern void Entity_SetVelocityXFromFacing(struct EwramData_EntityData *, s32);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *, s32, s32, s32);
extern void Bat_UpdateState4Animation(struct EwramData_EntityData *);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);


/**
 * @brief Face the target; choose waiting animations until the target enters the lower detection rectangle.
 */
void Bat_UpdateWaiting(struct EwramData_EntityData *entity)
{
    Entity_FaceTarget(entity);
    switch (entity->subState)
    {
    case 0:
        if (entity->animScriptIndex != 0) Anim_StartIndexed(entity, 0, 0, 1);
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        /* fall through */
    case 1:
        if (Bat_IsTargetBelow(entity, 0xE0, 0xA0))
        {
            if (Bat_IsTargetBelow(entity, 0xC0, 0x70))
            {
                entity->state = 2;
                entity->stage = 0;
                entity->subState = 0;
            }
            else if (entity->animScriptIndex != 1) Anim_StartIndexed(entity, 1, 0, 1);
        }
        else if (!(RandomNumberGenerator() & 0x7F))
        {
            entity->subState = 2;
            entity->stage = 0;
        }
        else if (entity->animScriptIndex != 0) Anim_StartIndexed(entity, 0, 0, 1);
        break;
    case 2:
        switch (entity->stage)
        {
        case 0:
            if (entity->animScriptIndex != 1) Anim_StartIndexed(entity, 1, 0, 1);
            entity->stage++;
            /* fall through */
        case 1:
            if (entity->animDone)
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        }
        break;
    }
}

/**
 * @brief Run the retreat, approach and sinusoidal flight substates; integrate velocity and acceleration.
 */
void Bat_UpdateFlight(struct EwramData_EntityData *entity)
{
    s32 distance;
    switch (entity->subState)
    {
    case 0:
        switch (entity->stage)
        {
        case 0:
            if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 1);
            entity->velX.whole = 0x8000;
            entity->velY.whole = 0x2000;
            Entity_FaceTarget(entity);
            Entity_SetVelocityXFromFacing(entity, entity->velX.whole);
            entity->velX.whole = -entity->velX.whole;
            entity->timer = 0x20;
            entity->stage++;
            /* fall through */
        case 1:
            if ((u8)--entity->timer == 0xFF)
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        }
        break;
    case 1:
        switch (entity->stage)
        {
        case 0:
            entity->velX.whole = 0x6000;
            entity->velY.whole = 0x6000;
            Entity_SetVelocityXFromFacing(entity, 0x6000);
            entity->stage++;
            /* fall through */
        case 1:
            Anim_Update(entity);
            distance = (s16)Entity_GetTargetDistanceY(entity);
            if (distance < 0) distance = -distance;
            if (distance <= 0x27)
            {
                entity->subState = 2;
                entity->stage = 0;
            }
            break;
        }
        break;
    case 2:
        switch (entity->stage)
        {
        case 0:
            entity->velX.whole = 0xC000;
            entity->velY.whole = 0x200;
            entity->accelX.whole = 0x50;
            entity->accelY.whole = 0x25;
            if (!entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
            }
            entity->unk_4F8.part16.unk_4F8 = 0;
            entity->stage++;
            /* fall through */
        case 1:
            entity->posY.whole += Sin(entity->unk_4F8.part16.unk_4F8) / 4;
            entity->unk_4F8.part16.unk_4F8 += 0x200;
            if ((s16)entity->posX.part16.integer < (s16)entity->velX.part16.integer - 0x28
                || (s16)entity->posX.part16.integer > (s16)entity->velX.part16.integer + 0xF0)
                entity->deleteRequested = 1;
            break;
        }
        break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

/**
 * @brief Drop rewards once, invert the sprite, fall and run the timed fragment effect.
 */
void Bat_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Enemy_DropRewards(entity);
        PlaySong(0x72);
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->animStep = 2;
        Anim_Update(entity);
        entity->velY.whole = 0x8000;
        entity->flipY = 1;
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x40;
        entity->subState++;
        /* fall through */
    case 1:
        entity->posY.whole += entity->velY.whole;
        Enemy_UpdateFragmentDeathWithSpread(entity, 0xF, 0x80000, 0x60000);
        break;
    }
}

void EnemyBatUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 4) Bat_UpdateState4Animation(entity);
    else if (entity->state == 3) Bat_UpdateDeath(entity);
    else if (!Enemy_PreUpdate(entity))
    {
        switch (entity->state)
        {
        case 1: Bat_UpdateWaiting(entity); break;
        case 2: Bat_UpdateFlight(entity); break;
        case 3: Bat_UpdateDeath(entity); break;
        }
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}
