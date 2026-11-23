/*
 * Nightmare.
 */
#include "core.h"
#include "bg_and_collision.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "sound.h"
#include "structs/ewram.h"

/* Nightmare running, attack and death states. */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void Nightmare_Draw(struct EwramData_EntityData *);
extern void Nightmare_OnHit(struct EwramData_EntityData *);
extern void Entity_HideAboveY(struct EwramData_EntityData *, s32);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern void Nightmare_UpdateRun(struct EwramData_EntityData *);
extern s32 Nightmare_UpdateDeath(struct EwramData_EntityData *);
extern u8 sNightmareGfx[], sNightmarePalette[], sNightmareSpriteSheet[];

extern void Entity_EndIfActive(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern u8 sNightmareWalkProfile[];

/**
 * @brief Alive state (1): accelerate, brake with particles and reverse
 * direction. Select animation and hit box changes from the entity variant.
 */
void Nightmare_UpdateRun(struct EwramData_EntityData *entity)
{
    s32 timer;
    struct EwramData_EntityData *effect;
    Entity_Walk(entity, entity->ext.nightmare.runSpeed, sNightmareWalkProfile, 4);
    switch (entity->subState)
    {
    case 0:
        entity->ext.nightmare.runSpeed = 0;
        entity->subState = 1;
        /* fall through */
    case 1:
        if ((u8)((u32)entity->timer % 25) == 0 && Entity_IsInRect(entity, -0x40, -0x40, 0x170, 0x120)) PlaySong(0x7D);
        entity->timer++;
        if ((s32)entity->ext.nightmare.runSpeed <= 0x27FFF) entity->ext.nightmare.runSpeed += 0x4000;
        if (++entity->ext.nightmare.runTimer > 0x50)
        {
            Anim_StartIndexed(entity, (u16)(entity->spawnParam0 + 2), 0, 0);
            if (entity->spawnParam0 == 0)
            {
                HitBox_SetBodyRaw(entity, 0x1810E0F8);
                HitBox_SetAttackRaw(entity, 0x1810E0F8);
            }
            PlaySong(0x127);
            entity->subState = 2;
            entity->timer = 0;
        }
        break;
    case 2:
        if ((s32)entity->ext.nightmare.runSpeed > 0) entity->ext.nightmare.runSpeed -= 0x1000;
        timer = entity->timer;
        timer++;
        entity->timer = timer;
        if (!(timer & 3))
        {
            s32 x = entity->posX.whole;
            s32 y = entity->posY.whole;
            effect = Effect_SpawnShared(x, y, (RandomNumberGenerator() & 1) + 4, 6);
            if (effect != NULL)
            {
                effect->drawSkip = 1;
                effect->velX.whole = 0x8000;
                if (entity->flipX) effect->velX.whole = -effect->velX.whole;
            }
        }
        if (entity->animFlags & 4)
        {
            entity->ext.nightmare.runTimer = 0;
            Anim_StartIndexed(entity, (u16)entity->spawnParam0, 0, 1);
            if (entity->spawnParam0 == 0)
            {
                HitBox_SetBodyRaw(entity, 0x1838E0F0);
                HitBox_SetAttackRaw(entity, 0x1838E0F0);
            }
            entity->subState = 0;
            entity->flipX ^= 1;
        }
        break;
    }
}

/**
 * @brief Death state (2): stop the two stored afterimage entities, launch
 * backward and upward, flash and emit particles. Return nonzero on the
 * existing bounds or animation/timer removal condition.
 */
s32 Nightmare_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 timer;
    s32 x;
    s32 y;
    switch (entity->subState)
    {
    case 0:
        if ((u8)entity->ext.nightmareEffect.index0 != 0)
        {
            Entity_EndIfActive(&gEwramData->entityData[(u8)entity->ext.nightmareEffect.index0]);
            entity->ext.nightmareEffect.index0 = 0;
        }
        if (entity->ext.nightmareEffect.index1 != 0)
        {
            Entity_EndIfActive(&gEwramData->entityData[entity->ext.nightmareEffect.index1]);
            entity->ext.nightmareEffect.index1 = 0;
        }
        Anim_StartIndexed(entity, (u16)(entity->spawnParam0 + 2), 0, 0);
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
        timer = entity->timer;
        timer++;
        entity->timer = timer;
        if (!(timer & 7)) PlaySong(0x6E);
        if (!(entity->timer & 3))
        {
            s32 offsetX = RandomSigned(0x10) << 16;
            s32 offsetY;
            x = entity->posX.whole;
            x += offsetX;
            offsetY = RandomSigned(0x10);
            offsetY -= 0x20;
            offsetY <<= 16;
            y = entity->posY.whole;
            y += offsetY;
            Effect_SpawnShared(x, y, 1, 7);
        }
        EntityAccelerateAndMove(entity, 0x80000);
        if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) || ((entity->animFlags & 4) && entity->timer <= 0x80)) return 1;
        break;
    }
    return 0;
}

/* Nightmare initialisation and state dispatch. */
void EnemyNightmareCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sNightmareGfx, sNightmarePalette, 0, sNightmareSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 9;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->spawnParam0 = 1;
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        entity->state = 3;
        return;
    }
    Anim_StartIndexed(entity, (u16)entity->spawnParam0, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Nightmare_Draw;
    if (entity->spawnParam0 == 0)
    {
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)Nightmare_OnHit);
        HitBox_SetBodyRaw(entity, 0x1838E0F0);
        HitBox_SetAttackRaw(entity, 0x1838E0F0);
    }
    else
    {
        Hit_EnableContact(entity, 8, Enemy_OnContact, (s32)Nightmare_OnHit);
        HitBox_SetBodyRaw(entity, 0x1838E0F0);
        HitBox_SetAttackRaw(entity, 0x2010C400);
    }
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
    entity->ext.nightmare.runTimer = 0x28;
}

void EnemyNightmareUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 enabled = 1;
    struct EwramData_EntityData *effect;
    entity->ext.nightmare.roomX = GetEntityRoomXPositionInteger(entity);
    entity->ext.nightmare.roomY = GetEntityRoomYPositionInteger(entity);
    Entity_HideAboveY(entity, -0x20);
    if (!(entity->enemyFlags & 2))
    {
        if (!Entity_IsInRect(entity, -0x100, -0x60, 0x2F0, 0x160)) return;
        Player_TrackEntity(entity);
        if (entity->spawnParam0 == 0)
        {
            enabled = Entity_IsTimeStopped(entity) == 0;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state == 2) return;
        Hit_CheckWithPlayer(box, entity);
        if (entity->spawnParam0 == 0) Player_QueueEntity(entity, 0);
        return;
    }
    if (entity->spawnParam0 == 0 && entity->state == 1) Player_QueueEntity(entity, enabled);
    switch (entity->state)
    {
    case 0:
        effect = Afterimage_Create(entity, 0, 0xC, 1);
        if (effect != NULL) entity->ext.nightmareEffect.index0 = ENTITY_INDEX(effect);
        effect = Afterimage_Create(entity, 0, 6, 0);
        if (effect != NULL) entity->ext.nightmareEffect.index1 = ENTITY_INDEX(effect);
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        /* fall through */
    case 1:
        Nightmare_UpdateRun(entity);
        break;
    case 2:
        if (Nightmare_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 2) Hit_CheckWithPlayer(box, entity);
    Anim_Update(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Nightmare_OnHit = HeadStackSegment_OnHit */

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 2 with the substate counters reset.
 */
void Nightmare_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/* Nightmare_Draw. */
/**
 * @brief Draw at the cached room coordinates relative to background 1,
 * then restore both full position values.
 */
void Nightmare_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    entity->posX.part16.integer = entity->ext.nightmare.roomX - bg->xPos.part16.integer;
    entity->posY.part16.integer = entity->ext.nightmare.roomY - bg->yPos.part16.integer;
    Sprite_DrawEntity(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
