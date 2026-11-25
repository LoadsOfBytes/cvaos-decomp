/*
 * Bat: create (placed on the ceiling unless time is frozen), waiting / flight / death states and hits.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include "entity_util.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "entity_inline.h"
#include "bg_and_collision.h"


void Bat_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData);
s32 Bat_IsTargetBelow(struct EwramData_EntityData *, u16, u16);
void Bat_UpdateState4Animation(struct EwramData_EntityData *);

/* States of Bat (entity->state), named after the update function the dispatcher calls. */
enum {
    BAT_STATE_WAITING = 1,
    BAT_STATE_FLIGHT = 2,
    BAT_STATE_DEATH = 3,
};

void EnemyBatCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBatPalette;
    u8 *sprite = sBatSpriteSheet;
    u8 *graphics = sBatGfx;
    s32 hit;
    s16 *y;
    s16 *py;
    s32 x;
    if (!Entity_LoadGraphics(entity, graphics, palette, 0, sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    hit = *((u8 *)&gEwramData->gameState + 0x2E) & 0x40;
    if (hit > 0)
    {
        Entity_SetStateAndStage(entity, 0, 4);
    }
    else
    {
        Entity_SetStateAndStage(entity, 0, 1);
        y = &entity->posY.part16.integer;
        py = y;
        do
        {
            x = GetEntityRoomXPositionIntegerWide(entity);
            hit = Collision_GetPushDownWide(x, GetEntityRoomYPositionIntegerWide(entity));
            if (hit == 0)
                *py -= 8;
        } while (hit == 0);
        *y += hit;
        Entity_EnableBoxes(entity, Enemy_OnContact, Bat_OnHit);
    }
}

/* Bat state machine, reconstructed from original ROM assembly. */

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
        entity->state = BAT_STATE_WAITING;
        entity->stage = 0;
        entity->subState = 0;
        /* fall through */
    case 1:
        if (Bat_IsTargetBelow(entity, 0xE0, 0xA0))
        {
            if (Bat_IsTargetBelow(entity, 0xC0, 0x70))
            {
                entity->state = BAT_STATE_FLIGHT;
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
            distance = Entity_GetTargetDistanceY(entity);
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
            entity->ext.bat.bobPhase = 0;
            entity->stage++;
            /* fall through */
        case 1:
            entity->posY.whole += Sin(entity->ext.bat.bobPhase) / 4;
            entity->ext.bat.bobPhase += 0x200;
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
        entity->palette = entity->basePalette;
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
    else if (entity->state == BAT_STATE_DEATH) Bat_UpdateDeath(entity);
    else if (!Enemy_PreUpdate(entity))
    {
        switch (entity->state)
        {
        case BAT_STATE_WAITING: Bat_UpdateWaiting(entity); break;
        case BAT_STATE_FLIGHT: Bat_UpdateFlight(entity); break;
        case BAT_STATE_DEATH: Bat_UpdateDeath(entity); break;
        }
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

/* Bat helpers 0x080AD6AC..0x080AD747. */

/**
 * @brief Start animation 1 once, then advance it. The gameplay meaning of state 4 is unresolved.
 */
void Bat_UpdateState4Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 1;
            entity->stage = 0;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Apply damage and enter death state 3 when Enemy_TakeDamage reports death.
 */
void Bat_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData)
{
    if (Enemy_TakeDamage(entity, attacker, hitData))
    {
        entity->state = BAT_STATE_DEATH;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Test playerState.enemyTarget in a width-by-height rectangle below the bat, centered horizontally.
 */
s32 Bat_IsTargetBelow(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    s16 left = entity->posX.part16.integer - (width >> 1);
    s16 top = entity->posY.part16.integer;

    return Entity_IsInRect(target, left, top, width, height);
}
