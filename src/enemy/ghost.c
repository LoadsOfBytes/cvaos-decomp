/*
 * Ghost.
 */
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "structs/entity_template.h"
#include "gameplay.h"
#include "syscalls.h"
#include "sound.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"

/* Update a Ghost or its timed spawn controller. */
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Ghost_UpdateFloat(struct EwramData_EntityData *);
extern void Ghost_UpdatePreview(struct EwramData_EntityData *);
extern void EnemyGhostCreate(struct EwramData_EntityData *);
void EnemyGhostUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 3)
    {
        if (entity->state != 1)
        {
            if (EntityIsOutsideScreen(entity, 4)) return;
            if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            s32 zero = 0;
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 1) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity))
    {
        entity->velX.whole = 0;
        if (entity->state != 1)
        {
            Hit_CheckWithPlayer(collision, entity);
            return;
        }
    }
    switch (entity->state)
    {
    case 0: Ghost_UpdateFloat(entity); break;
    case 1:
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            struct EwramData_EntityData *controller = ENTITY_OWNER(entity);
            if (controller && controller->updateFunc && controller->enemyId == 14
                && controller->ext.ghostSpawner.childCount != 0)
                controller->ext.ghostSpawner.childCount--;
        }
        break;
    case 2:
        Ghost_UpdatePreview(entity);
        Anim_Update(entity);
        return;
    case 3:
    {
        s32 spawn = 0;
        if (entity->ext.ghostSpawner.spawnDelay++ >= entity->spawnParam1
            && (RandomNumberGenerator() & 31) == 0)
        {
            entity->ext.ghostSpawner.spawnDelay = spawn;
            spawn = 1;
        }
        if (spawn && entity->ext.ghostSpawner.childCount < entity->spawnParam0)
        {
            struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child)
            {
                s32 distance;
                entity->ext.ghostSpawner.childCount++;
                do
                {
                    child->posX.part16.integer = RandomNumberGenerator() % 240;
                    distance = gEwramData->playerState.player->posX.whole - child->posX.whole;
                    if (distance < 0) distance = -distance;
                } while (distance <= 0x300000);
                do
                {
                    child->posY.part16.integer = RandomNumberGenerator() % 176 + 16;
                    distance = gEwramData->playerState.player->posY.whole - child->posY.whole;
                    if (distance < 0) distance = -distance;
                } while (distance <= 0x300000);
                child->scrollWithBg = 1;
                child->spawnIndex = 0;
                child->enemyId = entity->enemyId;
                child->spawnParam1 = 0;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                child->hp = sEnemyData[entity->enemyId].hp;
                child->mp = sEnemyData[entity->enemyId].mp;
                child->updateFunc = entity->updateFunc;
                EnemyGhostCreate(child);
                ENTITY_OWNER(child) = entity;
            }
        }
        return;
    }
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Initialize a Ghost, a hidden controller or the display-mode animation. */
extern u8 Ghost_LoadGraphics(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
void Ghost_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *other);
void Ghost_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void EnemyGhostCreate(struct EwramData_EntityData *entity)
{
    struct SpriteBox box = {-7, -7, 14, 14};
    if (entity->spawnParam1 != 0)
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        return;
    }
    if (!Ghost_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 0);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        entity->posY.whole -= 0x240000;
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Ghost_DamagePlayerOnContact, Ghost_OnHit);
    else
        Hit_Enable(entity, 8, Ghost_DamagePlayerOnContact, (s32)Ghost_OnHit);
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    entity->hit.flags |= 2;
    entity->hit.flags |= 4;
    entity->state = 0;
    entity->flipX = Entity_IsTargetRight(entity);
}

/* Appear, follow a target with damping, then choose a new outward direction. */
static inline s32 Magnitude(s32 value)
{
    if (value < 0) return -value;
    return value;
}
static inline s32 TargetOffsetY(struct EwramData_EntityData *entity, s32 x)
{
    if (Magnitude(x) < 0x600000)
    {
        s32 distance = Target_GetY(entity) - entity->posY.whole;
        s32 offset = entity->ext.ghost.aimOffsetY - 0x100000;
        return distance + offset;
    }
    return Target_GetY(entity) - entity->posY.whole - 0x200000;
}
/**
 * @brief Float state (state 0): after the appear animation become hittable, then
 * drift toward the target with damped acceleration; when within 4 px bounce
 * away in a random direction (+-0x1000 around the angle from the target).
 */
void Ghost_UpdateFloat(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (entity->animDone)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->hit.flags &= ~2;
            entity->hit.flags &= ~4;
            entity->ext.ghost.aimOffsetY = -(RandomNumberGenerator() & 0xFFFFF);
        }
        break;
    case 1:
    {
        s32 x, y;
        entity->flipX = Entity_IsTargetRight(entity);
        x = Target_GetX(entity) - entity->posX.whole;
        y = TargetOffsetY(entity, x);
        if (x > 0x40000) entity->accelX.whole = 0x400;
        else if (x < -0x40000) entity->accelX.whole = -0x400;
        else entity->accelX.whole = 0;
        if (y > 0x40000) entity->accelY.whole = 0x400;
        else if (y < -0x40000) entity->accelY.whole = -0x400;
        else entity->accelY.whole = 0;
        EntityDampVelocityX(entity, 32);
        EntityDampVelocityY(entity, 32);
        EntityAccelerateAndMove(entity, 0x10000);
        if (Magnitude(x) < 0x40000 && Magnitude(y) < 0x40000)
        {
            s32 pixelX, pixelY;
            s32 parameter;
            x = entity->posX.whole - Target_GetX(entity);
            y = entity->posY.whole - Target_GetY(entity) + 0x100000;
            if (x < 0) pixelX = (s16)(-((-x) >> 16));
            else pixelX = x >> 16;
            if (y < 0) pixelY = (s16)(-((-y) >> 16));
            else pixelY = y >> 16;
            parameter = ArcTan2(pixelX, pixelY);
            {
                s32 random = RandomNumberGenerator();
                s32 base = parameter - 0x1000;
                parameter = base + (random & 0x1FFF);
            }
            x = Sin(parameter + 0x4000);
            y = Sin(parameter);
            /* Reuse the consumed angle's temporary for the fixed-point speed. */
            parameter = 0x20000;
            entity->velX.whole = FixedMultiply(x, parameter);
            entity->velY.whole = FixedMultiply(y, parameter);
        }
        break;
    }
    }
}

/* Bounce away from contact or a hit, using the direction between entities. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
static inline void Bounce(struct EwramData_EntityData *entity, struct EwramData_EntityData *other)
{
    s32 x = entity->posX.whole - other->posX.whole;
    s32 y = entity->posY.whole - other->posY.whole;
    s32 pixelX, pixelY;
    u16 direction;
    s32 parameter;
    if (x < 0) pixelX = (s16)(-((-x) >> 16));
    else pixelX = x >> 16;
    if (y < 0) pixelY = (s16)(-((-y) >> 16));
    else pixelY = y >> 16;
    direction = ArcTan2(pixelX, pixelY);
    parameter = direction;
    x = Sin(parameter + 0x4000);
    y = Sin(parameter);
    /* The direction has been consumed; reuse the temporary for speed. */
    parameter = 0x20000;
    entity->velX.whole = FixedMultiply(x, parameter);
    entity->velY.whole = FixedMultiply(y, parameter);
}
/**
 * @brief Contact: Player_ApplyContactDamage with (attack, 0, 0, 0), then bounce
 * off the other entity.
 */
void Ghost_DamagePlayerOnContact(struct EwramData_EntityData *entity, struct EwramData_EntityData *other)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    Bounce(entity, other);
}

/* Bounce away from contact or a hit, using the direction between entities. */
/**
 * @brief Hit: resolve and show the damage, flash and bounce off the attacker; at
 * HP <= 0 drop the rewards and die (song 0x6E, state 1).
 */
void Ghost_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;
        damage = (s16)damage;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    Bounce(entity, attacker);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->basePalette;
        entity->timer = 8;
        entity->state = 1;
        entity->subState = 0;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ghost_LoadGraphics = Object25_LoadGraphics */

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 8A1AC | Same as Object25_LoadGraphics.
 */
u8 Ghost_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sGhostGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sGhostPalette, 0);
    Sprite_Attach(entity, sGhostSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Enter the looping display animation once the first animation completes. */
/**
 * @brief Bestiary preview state (state 2): after the first animation loop
 * animation 1.
 */
void Ghost_UpdatePreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0 && entity->animDone)
    {
        entity->subState = 1;
        Anim_StartIndexed(entity, 1, 0, 1);
    }
}
