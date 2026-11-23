/*
 * Merman: update, create, movement, attack, hits, shots and graphics.
 * Built with -fno-gcse (Merman_UpdateMove needs it; the rest compiles the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "player_core.h"
#include "entity_util.h"
#include "structs/entity_template.h"
#include "structs/ewram.h"
#include "sound.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"

/* Dispatch Merman states, including the hidden instance's child spawning. */
extern void EnemyMermanCreate(struct EwramData_EntityData *);
extern void Merman_UpdateMove(struct EwramData_EntityData *);
extern void Merman_UpdateAttack(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
void EnemyMermanUpdate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u8 collision[16];
    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
        else entity->hidden = 0;
        if (Entity_IsOutsideRoomMap(entity))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2) Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: Merman_UpdateMove(entity); break;
    case 1: Merman_UpdateAttack(entity); break;
    case 2:
        {
            struct EwramData_EntityData *parent;
            Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 0);
            Enemy_UpdateFragmentDeath(entity);
            if (entity->deleteRequested)
            {
                parent = entity->ext.merman.spawner;
                if (parent && parent->updateFunc && parent->enemyId == 3)
                {
                    if (parent->ext.mermanSpawner.childCount) parent->ext.mermanSpawner.childCount--;
                }
            }
        }
        break;
    case 3:
        {
            s32 zero = RandomNumberGenerator() & 7;
            struct EwramData_EntityData *child;
            s32 mode;
            if (zero != 0) goto finished;
            if ((s32)entity->ext.mermanSpawner.childCount > 2) goto finished;
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child == NULL) goto finished;
            mode = entity->spawnParam0;
            if (mode != 1) goto finished;
            entity->ext.mermanSpawner.childCount++;
            child->posX.whole = (((RandomNumberGenerator() & 0xFF) + 144) << 16) - background->xPos.whole;
            child->posY.whole = ((entity->ext.merman.homeY + 32) << 16) - background->yPos.whole;
            child->scrollWithBg = 1;
            child->spawnIndex = zero;
            child->enemyId = entity->enemyId;
            child->spawnParam0 = mode;
            child->spawnParam1 = zero;
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            child->hp = sEnemyData[entity->enemyId].hp;
            child->mp = sEnemyData[entity->enemyId].mp;
            child->updateFunc = entity->updateFunc;
            EnemyMermanCreate(child);
            entity->timer = 60;
            child->ext.merman.spawner = entity;
            child->ext.merman.homeX = entity->ext.merman.homeX;
            child->ext.merman.homeY = entity->ext.merman.homeY;
        }
        goto finished;
    case 4: Anim_Update(entity); goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}

/* Initialize Merman's hidden, waiting or jumping entry mode. */
extern u8 Merman_LoadGraphics(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_DamagePlayerOnContact(struct EwramData_EntityData *);
void Merman_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void EnemyMermanCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (!Merman_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        goto finished;
    }
    Entity_EnableBoxes(entity, Enemy_DamagePlayerOnContact, Merman_OnHit);
    {
        s32 mode = entity->spawnParam1;
        if (mode != 0)
        {
            entity->state = 3;
            entity->hidden = 1;
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            entity->ext.merman.homeX = entity->posX.part16.integer + background->xPos.part16.integer;
            entity->ext.merman.homeY = entity->posY.part16.integer + background->yPos.part16.integer;
        }
        else
        {
            entity->state = mode;
            entity->timer = 60;
            if (entity->spawnParam0 == 0)
            {
                Entity_SnapToGround(entity);
                entity->subState = 10;
                entity->accelX.whole = mode;
                entity->velY.whole = 0x20000;
                entity->accelY.whole = 0x1800;
                entity->ext.merman.leapSplashed = mode;
                entity->ext.merman.swimMode = mode;
                entity->ext.merman.landSplashed = mode;
                entity->ext.merman.walkCount = mode;
                Anim_StartIndexed(entity, 2, 0, 1);
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
                entity->timer = 0;
                entity->ext.merman.unusedMode = 1;
            }
            else entity->ext.merman.unusedMode = mode;
        }
    }
finished:
    return;
}

/* Merman movement and transitions between swimming, jumping and landing.
 * This reconstruction requires the source-specific -fno-gcse build rule.
 * A byte match establishes generated code, not the original compiler options. */
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
/* Preserve the original caller's full-register argument arithmetic. The callee
 * consumes signed 16-bit coordinates; its implementation performs narrowing. */
extern s32 Collision_FindWaterSurfaceOffset(s32, s32);
#define LEAP_SPLASHED(e) (e->ext.merman.leapSplashed)
#define SWIM_MODE(e) (*(s8 *)&e->ext.merman.swimMode)
#define LAND_SPLASHED(e) (*(s8 *)&e->ext.merman.landSplashed)
#define WALK_COUNT(e) (e->ext.merman.walkCount)
#define HOME_Y(e) ((s16)e->ext.merman.homeY)
#define ACCEL_X(e) ((e)->accelX.whole)
#define ACCEL_Y(e) ((e)->accelY.whole)
#define ROOM_Y(e,c) ((e)->posY.part16.integer + (s16)(c)->yPos.part16.integer)

/**
 * @brief Movement state machine (state 0): swim, jump out of the water, land and
 * walk; starts the shot attack (state 1).
 */
void Merman_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 flags;
restart:
    switch (entity->subState)
    {
    case 0:
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            entity->subState++;
            entity->velX.whole = 0;
            entity->velY.whole = -0x8000;
            ACCEL_X(entity) = 0;
            ACCEL_Y(entity) = 0;
        }
        break;
    case 1:
        EntityMoveY(entity);
        if (ROOM_Y(entity, camera) < HOME_Y(entity)) EntityDampVelocityY(entity, 16);
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->velY.whole > -0x4000) entity->subState++;
        if (RandomNumberGenerator() & 1)
        {
            SWIM_MODE(entity) = 0;
            Anim_StartIndexed(entity, 4, 0, 1);
        }
        else
        {
            SWIM_MODE(entity) = 1;
            Anim_StartIndexed(entity, 5, 0, 1);
            entity->flipX = Entity_IsTargetRight(entity);
        }
        break;
    case 2:
    {
        s32 random = RandomNumberGenerator() & 3;
        if (random)
        {
            if (SWIM_MODE(entity) == 0)
            {
                ACCEL_Y(entity) = 0;
                if (ROOM_Y(entity, camera) < HOME_Y(entity) - 24)
                    entity->velY.whole = 0x8000;
                else if (ROOM_Y(entity, camera) <= HOME_Y(entity) + 8 && (RandomNumberGenerator() & 1))
                    entity->velY.whole = 0x8000;
                else entity->velY.whole = -0x8000;
                ACCEL_X(entity) = 0;
                if (RandomNumberGenerator() & 1) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
            }
            else
            {
                entity->flipX = Entity_IsTargetRight(entity);
                entity->velX.whole = 0;
                ACCEL_X(entity) = 0;
                ACCEL_Y(entity) = 0;
                if (ROOM_Y(entity, camera) < HOME_Y(entity) - 24)
                    entity->velY.whole = 0x8000;
                else if (ROOM_Y(entity, camera) <= HOME_Y(entity) + 8 && (RandomNumberGenerator() & 1))
                    entity->velY.whole = 0x8000;
                else entity->velY.whole = -0x8000;
            }
            entity->subState++;
            entity->timer = 0;
            goto restart;
        }
        else
        {
            s32 distance = (s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
            if (distance < 0) distance = -distance;
            if (distance <= 32)
            {
                entity->velY.whole = 0x8000;
                if (RandomNumberGenerator() & 1) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
                ACCEL_X(entity) = 0;
                ACCEL_Y(entity) = 0;
                entity->timer = 0;
                entity->subState = 4;
            }
            else
            {
                entity->velX.whole = random;
                ACCEL_X(entity) = random;
                entity->velY.whole = -0x54000;
                ACCEL_Y(entity) = 0x2800;
                entity->subState = 5;
                Anim_StartIndexed(entity, 0, 0, 1);
                LEAP_SPLASHED(entity) = random;
                SWIM_MODE(entity) = random;
                LAND_SPLASHED(entity) = random;
            }
        }
        break;
    }
    case 3:
        if (entity->timer++ > 31) entity->subState = 2;
        /* fall through */
    case 4:
        EntityMove(entity);
        if ((s16)entity->spawnParam0 == 1)
        {
            s32 roomX = (s16)(entity->posX.part16.integer + camera->xPos.part16.integer);
            /* Narrow the partial sum before subtraction to retain agbcc ordering. */
            if (roomX < 136) entity->posX.part16.integer = (u16)(entity->posX.part16.integer + 136) - roomX;
            else if (roomX > 408) entity->posX.part16.integer = (u16)(entity->posX.part16.integer + 408) - roomX;
        }
        break;
    case 5:
    {
        flags = Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 1);
        if (entity->velY.whole < 0 && LEAP_SPLASHED(entity) == 0 && ROOM_Y(entity, camera) < HOME_Y(entity) && !(flags & 0x80))
        {
            s32 offset;
            LEAP_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 32);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x200000;
                adjusted;
            }), entity->spriteLayer - 1);
        }
        if (entity->velY.whole > 0 && LAND_SPLASHED(entity) == 0 && (flags & 0x40))
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
            entity->subState = 9;
        }
        if (flags & 4)
        {
            entity->subState = 6;
            ACCEL_X(entity) = 0;
            entity->velY.whole = 0x20000;
            ACCEL_Y(entity) = 0x1800;
            LEAP_SPLASHED(entity) = 0;
            SWIM_MODE(entity) = 0;
            LAND_SPLASHED(entity) = 0;
            WALK_COUNT(entity) = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->flipX = Entity_IsTargetRight(entity);
            if (entity->flipX) entity->velX.whole = 0x8000;
            else entity->velX.whole = -0x8000;
            entity->timer = 0;
        }
        break;
    }
    case 6:
    {
        flags = Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 0);
        if ((flags & 0x44) == 0x40 && LAND_SPLASHED(entity) == 0)
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
            entity->subState = 9;
        }
        else if (!(flags & 4)) entity->subState = 9;
        else if (entity->timer++ > 59)
        {
            entity->timer = 0;
            if ((s8)++WALK_COUNT(entity) > 8)
            {
                entity->velX.whole = 0;
                ACCEL_X(entity) = 0;
                entity->velY.whole = -0x30000;
                ACCEL_Y(entity) = 0x1800;
                entity->subState = 9;
                Anim_StartIndexed(entity, 0, 0, 1);
                LEAP_SPLASHED(entity) = 0;
                LAND_SPLASHED(entity) = 0;
            }
            else if (RandomNumberGenerator() & 3)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
            }
            else
            {
                entity->state = 1;
                entity->subState = 0;
            }
        }
        break;
    }
    case 9:
    {
        flags = Entity_MoveAndProbePointCollision(entity, 0x2800, -0x200000, 0);
        if (entity->velY.whole > 0 && LAND_SPLASHED(entity) == 0 && (flags & 0x40))
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
        }
        break;
    }
    case 10:
    {
        flags = Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 0);
        if ((flags & 0x44) == 0x40 && LAND_SPLASHED(entity) == 0)
        {
            s32 offset;
            LAND_SPLASHED(entity) = 1;
            offset = Collision_FindWaterSurfaceOffset((s16)entity->posX.part16.integer, entity->posY.part16.integer - 7);
            Effect_SpawnStationaryAnimation(entity->posX.whole, entity->posY.whole + ({
                /* Keep the offset calculation grouped after the X load. */
                s32 adjusted = (offset << 16) - 0x70000;
                adjusted;
            }), entity->spriteLayer - 1);
        }
        if (entity->timer++ > 59)
        {
            entity->timer = 0;
            if (!(flags & 4)) break;
            if (RandomNumberGenerator() & 3)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX) entity->velX.whole = 0x8000;
                else entity->velX.whole = -0x8000;
            }
            else
            {
                entity->state = 1;
                entity->subState = 0;
            }
        }
        if (flags & 4) LAND_SPLASHED(entity) = 0;
        break;
    }
    }
}

/* Play the Merman attack, emit its projectile and restart movement. */
extern void Merman_SpawnShot(struct EwramData_EntityData *);
static inline void RestartJump(struct EwramData_EntityData *entity)
{
    entity->state = 0;
    entity->subState = 10;
    entity->accelX.whole = 0;
    entity->velY.whole = 0x40000;
    entity->accelY.whole = 0x1800;
    entity->ext.merman.leapSplashed = 0;
    entity->ext.merman.swimMode = 0;
    entity->ext.merman.landSplashed = 0;
    Anim_StartIndexed(entity, 2, 0, 1);
    entity->flipX = Entity_IsTargetRight(entity);
    if (entity->flipX) entity->velX.whole = 0x8000;
    else entity->velX.whole = -0x8000;
    entity->timer = 0;
}
static inline void RestartSwim(struct EwramData_EntityData *entity)
{
    entity->state = 0;
    entity->subState = 6;
    entity->accelX.whole = 0;
    entity->velY.whole = 0x20000;
    entity->accelY.whole = 0x1800;
    entity->ext.merman.leapSplashed = 0;
    entity->ext.merman.swimMode = 0;
    entity->ext.merman.landSplashed = 0;
    Anim_StartIndexed(entity, 2, 0, 1);
    entity->flipX = Entity_IsTargetRight(entity);
    if (entity->flipX) entity->velX.whole = 0x8000;
    else entity->velX.whole = -0x8000;
    entity->timer = 0;
}
/**
 * @brief Attack state machine (state 1): animation 3 and, from its fourth step,
 * sound 0x7A and Merman_SpawnShot; then back to state 0.
 */
void Merman_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animStep > 3)
        {
            PlaySong(0x7A);
            Merman_SpawnShot(entity);
            entity->subState++;
        }
        break;
    case 2:
        if (entity->animDone)
        {
            if (entity->spawnParam0 == 0)
                RestartJump(entity);
            else
                RestartSwim(entity);
        }
        break;
    }
}

/* Apply a Merman hit and begin the falling death state when HP is exhausted. */
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
void Merman_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    u8 *flashTimer = &entity->hitFlashTimer;
    s32 zero = 0;
    *flashTimer = 8;
    {
        s32 hp = entity->hp;
        damage = (s16)damage;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->palette = entity->basePalette;
        entity->timer = 40;
        entity->state = 2;
        entity->velX.whole = zero;
        entity->accelX.whole = zero;
        entity->velY.whole = zero;
        entity->accelY.whole = 0x2000;
    }
}

/* Create a Merman projectile and inherit the parent's visual attributes. */
extern void MermanShot_Update(struct EwramData_EntityData *);
extern u8 MermanShot_LoadGraphics(struct EwramData_EntityData *, struct EwramData_EntityData *);
extern void MermanShot_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Spawn the horizontal shot 5 px in front of and 24 px above parent at
 * 2 px per frame in its facing direction.
 */
void Merman_SpawnShot(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *projectile = EntityCreateInRange(0x1B, 0x48, (u32 *)MermanShot_Update);
    if (projectile)
    {
        if (!MermanShot_LoadGraphics(parent, projectile)) EntityDelete(projectile);
        projectile->scrollWithBg = 1;
        MermanShot_EnableHitBoxes(projectile);
        projectile->spriteLayer = parent->spriteLayer + 1;
        ENTITY_CREATOR(projectile) = parent;
        projectile->flipX = parent->flipX;
        if (parent->flipX)
        {
            projectile->posX.part16.integer = parent->posX.part16.integer + 5;
            projectile->velX.whole = 0x20000;
        }
        else
        {
            projectile->posX.part16.integer = parent->posX.part16.integer - 5;
            projectile->velX.whole = -0x20000;
        }
        projectile->posY.part16.integer = parent->posY.part16.integer - 24;
        projectile->spawnIndex = parent->spawnIndex;
        projectile->basePalette = parent->basePalette;
        projectile->attack = parent->attack;
    }
}

/* MermanShot_EnableHitBoxes (Merman hit boxes). */
extern void MermanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void MermanShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief 7171C | Activate box slot 0xA with MermanShot_DamagePlayerOnContact / MermanShot_OnHit and
 * set both boxes to the bytes of 0x0810FCF8 (x -8, y -4, w 16, h 8 - not a
 * ROM address); clear flags bits 1 and 2.
 */
void MermanShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0810FCF8;

    Hit_Enable(entity, 0xA, MermanShot_DamagePlayerOnContact, (s32)MermanShot_OnHit);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags &= ~4;
}

/* Move the projectile or emit its two fragments before deleting it. */
/**
 * @brief Shot update: fly until off screen (state 0); state 2 bursts into two
 * random effects and deletes the shot.
 */
void MermanShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    switch (entity->state)
    {
    case 0:
        EntityMoveX(entity);
        if (!EntityIsOutsideScreen(entity, 1)) goto animate;
        goto remove;
    case 1: break;
    case 2:
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xA0000, 0x40000, entity->spriteLayer);
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xA0000, 0x40000, entity->spriteLayer);
        goto remove;
    }
    goto animate;
remove:
    EntityDelete(entity);
    goto finished;
animate:
    Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
finished:
    return;
}

/* Merman helpers 0x080717E0..0x08071903. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 717E0 | Give projectile Merman's graphics (palette row of entity),
 * animation 6; 0 when no graphics slot is free.
 */
u8 MermanShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sMermanGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sMermanSpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(projectile, 6, 0, 1);
    Anim_Update(projectile);
    projectile->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 71830 | Contact: Player_ApplyContactDamage(attack, 2, 0, 1), then state 2 with a
 * 0x28 frame timer.
 */
void MermanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 2;
    entity->timer = 0x28;
    entity->subState = 0;
}

/**
 * @brief 71870 | Hit: hit effect, then state 2 with a 0x28 frame timer.
 */
void MermanShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 2;
    entity->timer = 0x28;
    entity->subState = 0;
}

/**
 * @brief 7188C | Load the graphics of Merman; 0 when no slot is free.
 */
u8 Merman_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMermanGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sMermanPalette, 0);
    Sprite_Attach(entity, sMermanSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 718D8 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Enemy_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
