/*
 * Zombie: body, debris, pieces and temporary sprites.
 */
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "structs/entity_template.h"
#include "sound.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "entity_hit.h"

void Zombie_UpdateEmergingWalkingSinking(struct EwramData_EntityData *);
void Zombie_UpdateStationaryAnimation(struct EwramData_EntityData *);
void EnemyZombieCreate(struct EwramData_EntityData *);
u8 Zombie_LoadGraphics(struct EwramData_EntityData *);
void Zombie_DamagePlayerOnContact(struct EwramData_EntityData *);
void Zombie_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Zombie_SpawnFallingDebris(struct EwramData_EntityData *, s32, s32);
void Zombie_SpawnTemporarySprite(struct EwramData_EntityData *, s32, s32);
void Zombie_SpawnPiece(struct EwramData_EntityData *, s32, s32);
void ZombiePiece_Update(struct EwramData_EntityData *);
void ZombieDebris_Update(struct EwramData_EntityData *);
void ZombieTemporarySprite_Update(struct EwramData_EntityData *);

/* Update a Zombie or its controller that maintains a population of children. */
void EnemyZombieUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state != 3)
    {
        if (entity->state != 1 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            s32 zero = 0;
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 1) Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Zombie_UpdateEmergingWalkingSinking(entity);
        break;
    case 1:
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            struct EwramData_EntityData *controller = entity->ext.zombie.spawner;
            if (controller && controller->updateFunc && controller->enemyId == 1
                && controller->ext.zombieSpawner.childCount != 0)
                controller->ext.zombieSpawner.childCount--;
        }
        break;
    case 2:
        Zombie_UpdateStationaryAnimation(entity);
        Anim_Update(entity);
        return;
    case 3:
        if ((RandomNumberGenerator() & 31) == 0
            && (s32)entity->ext.zombieSpawner.childCount < entity->spawnParam0)
        {
            struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
            if (child)
            {
                s32 distance;
                entity->ext.zombieSpawner.childCount++;
                do
                {
                    child->posX.part16.integer = RandomNumberGenerator() % 240;
                    distance = gEwramData->playerState.player->posX.whole - child->posX.whole;
                    if (distance < 0) distance = -distance;
                } while (distance <= 0x200000);
                child->posY.whole = entity->posY.whole;
                child->scrollWithBg = 1;
                child->spawnIndex = 0;
                child->enemyId = entity->enemyId;
                child->spawnParam1 = 0;
                child->drawFunc = Sprite_DrawEntity;
                child->hp = sEnemyData[entity->enemyId].hp;
                child->mp = sEnemyData[entity->enemyId].mp;
                child->updateFunc = entity->updateFunc;
                EnemyZombieCreate(child);
                child->ext.zombie.spawner = entity;
            }
        }
        return;
    }
    {
        u8 *frame = &entity->animFrame;
        u8 previous = *frame;
        Anim_Update(entity);
        if (previous != *frame) Entity_UpdateBoxes(entity);
    }
    if (entity->state == 0 && (entity->subState == 0 || entity->subState == 3))
        entity->hit.flags |= 2;
    else
        entity->hit.flags &= ~2;
    Enemy_CheckHitPlayer(entity);
}

/* Initialize a Zombie or its hidden spawn controller. */
void EnemyZombieCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (entity->spawnParam1 != 0)
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Entity_SnapToGround(entity);
        return;
    }
    if (!Zombie_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 0);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->state = 2;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Zombie_DamagePlayerOnContact, Zombie_OnHit);
    else
        Hit_Enable(entity, 8, Zombie_DamagePlayerOnContact, Zombie_OnHit);
    Entity_UpdateBoxes(entity);
    {
        s32 previousY = entity->posY.whole;
        s32 zero;
        Entity_SnapToGround(entity);
        zero = 0;
        entity->state = zero;
        entity->flipX = Entity_IsTargetRight(entity);
        if (Collision_IsSlopeTile((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(background->yPos.part16.integer + entity->posY.part16.integer)))
        {
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
        if (entity->posY.whole <= previousY - 0x80000)
        {
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
    }
}

/* Zombie_UpdateEmergingWalkingSinking: matched (timer accessed as a field, block-local hit for the two
   Target_IsInRect calls, u8 parameter of BeginSinking). */
/* Zombie's emerging, walking and sinking states. */
static inline void WalkVelocity(struct EwramData_EntityData *entity)
{
    if (entity->flipX) entity->velX.whole = 0x20000;
    else entity->velX.whole = -0x20000;
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x2800;
}
static inline void BeginSinking(struct EwramData_EntityData *entity, s32 x, s32 y, u8 zero)
{
    entity->subState++;
    entity->animFlags |= 2;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->timer = zero;
}
/**
 * @brief Wait for emergence, walk with collision, then pause and sink before entering removal state 1.
 */
void Zombie_UpdateEmergingWalkingSinking(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        if (entity->animDone)
        {
            entity->subState = 1;
            if (RandomNumberGenerator() & 1) Anim_StartIndexed(entity, 1, 0, 1);
            else Anim_StartIndexed(entity, 4, 0, 1);
            WalkVelocity(entity);
            entity->ext.zombie.timer = (RandomNumberGenerator() & 511) + 600;
        }
        break;
    case 1:
    {

        s32 previousX, previousY, impact;
        if (entity->animTimer == 0) WalkVelocity(entity);
        EntityDampVelocityX(entity, 4);
        previousX = entity->posX.whole;
        previousY = entity->posY.whole;
        impact = Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8) & 3;
        if (impact)
        {
            BeginSinking(entity, previousX, previousY, 0);
            break;
        }
        {
        s32 tile = Collision_IsSlopeTile((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                             (s16)(background->yPos.part16.integer + entity->posY.part16.integer));
        if (tile)
        {
            BeginSinking(entity, previousX, previousY, impact);
            break;
        }
        if (entity->ext.zombie.timer-- <= 0)
        {
            BeginSinking(entity, previousX, previousY, tile);
            break;
        }
        }
        if (entity->animStep == 0)
        {
            if (entity->ext.zombie.debrisSpawned == 0)
            {
                entity->ext.zombie.debrisSpawned = 1;
                if (entity->animScriptIndex == 4)
                {
                    previousX = entity->flipX ? -0x90000 : 0x90000;
                    previousY = -0x180000;
                }
                else
                {
                    previousX = entity->flipX ? 0x90000 : -0x90000;
                    previousY = -0x160000;
                }
                previousX += entity->posX.whole;
                previousY += entity->posY.whole;
                Zombie_SpawnFallingDebris(entity, previousX, previousY);
            }
        }
        if (entity->animStep == 0 || entity->animStep == 2)
        {
            if ((s8)entity->ext.zombie.spriteSpawned == 0)
            {
                entity->ext.zombie.spriteSpawned = 1;
                previousX = entity->posX.whole;
                previousY = entity->posY.whole;
                Zombie_SpawnTemporarySprite(entity, previousX, previousY);
            }
        }
        if (entity->animStep == 1 || entity->animStep == 3)
        {
            entity->ext.zombie.debrisSpawned = 0;
            entity->ext.zombie.spriteSpawned = 0;
        }
        {
            s32 hit;
            if (entity->flipX)
                hit = Target_IsInRect((s16)entity->posX.part16.integer, (s16)(entity->posY.part16.integer - 32), 80, 64);
            else
                hit = Target_IsInRect((s16)(entity->posX.part16.integer - 80), (s16)(entity->posY.part16.integer - 32), 80, 64);
            if (!hit) break;
        }
        if (!(entity->ext.zombie.timer & 1)) entity->animTimer++;
        break;
    }
    case 2:
        entity->animFlags |= 2;
        if (entity->timer++ > 59)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->timer = 0;
            entity->subState++;
        }
        break;
    case 3:
        if (entity->animDone)
        {
            s32 zero = 0;
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
        break;
    }
}

/* Select an animation and emit debris at its impact frame. */
/**
 * @brief Choose animation 1 or 4 after emergence and emit frame-13 debris at the animation event; no movement.
 */
void Zombie_UpdateStationaryAnimation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (entity->animDone)
        {
            entity->subState = 1;
            if (RandomNumberGenerator() & 1) Anim_StartIndexed(entity, 1, 0, 1);
            else Anim_StartIndexed(entity, 4, 0, 1);
        }
        break;
    case 1:
        if (entity->animStep == 0 && entity->animTimer == 0)
        {
            s32 x, y;
            if (entity->animScriptIndex == 4)
            {
                x = entity->flipX ? -0x90000 : 0x90000;
                y = -0x180000;
            }
            else
            {
                x = entity->flipX ? 0x70000 : -0x70000;
                y = -0x160000;
            }
            x += entity->posX.whole;
            y += entity->posY.whole;
            Zombie_SpawnFallingDebris(entity, x, y);
        }
        break;
    }
}

/* This inline step preserves the original order of the coordinate additions.
 * Combining it with the position expression changes constant reuse and registers. */
static inline s32 Offset(s32 value, s32 delta)
{
    value += delta;
    return value;
}

/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x70, animation 3, state 1), throwing two pieces
 * (Zombie_SpawnPiece).
 */
void Zombie_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x70);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
        {
            s32 x = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            s32 y;
            s32 r;
            r = RandomNumberGenerator() & 0x3FFFF;
            x += entity->posX.whole;
            y = Offset(r, -0x1A0000) + entity->posY.whole;
            Zombie_SpawnPiece(entity, x, y);
            x = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            r = RandomNumberGenerator() & 0x3FFFF;
            x += entity->posX.whole;
            y = Offset(r, -0x1A0000) + entity->posY.whole;
            Zombie_SpawnPiece(entity, x, y);
        }
    }
}

/* Zombie debris and temporary effects. */
/**
 * @brief Create frame-13 debris at fixed-point x/y, inheriting facing, layer and palette; lifetime 30 ticks.
 */
void Zombie_SpawnFallingDebris(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = Entity_CreateObject(ZombieDebris_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer;
        graphics = ObjGfx_Find(sZombieGfx);
        Sprite_Attach(part, sZombieSpriteSheet, graphics, parent->basePalette);
        part->animFrame = 13;
        part->timer = 30;
        part->accelY.whole = 0xA00;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Zombie debris and temporary effects. */
/**
 * @brief Spawn a flesh piece (frame 13) at (x, y) with a random upward velocity
 * that falls until it passes the parent's feet (ZombiePiece_Update).
 */
void Zombie_SpawnPiece(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, ZombiePiece_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->ext.zombiePiece.floorY = parent->posY.whole;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer;
        graphics = ObjGfx_Find(sZombieGfx);
        Sprite_Attach(part, sZombieSpriteSheet, graphics, parent->basePalette);
        part->animFrame = 13;
        part->accelY.whole = 0x1800;
        part->velX.whole = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        part->velY.whole = -0x10000 - (RandomNumberGenerator() & 0xFFFF);
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Zombie debris and temporary effects. */
/**
 * @brief Apply vertical acceleration and movement; delete when the signed byte timer expires.
 */
void ZombieDebris_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateY(entity, 0x80000);
    EntityMoveY(entity);
    if ((s8)entity->timer-- <= 0) EntityDelete(entity);
}

/* Zombie debris and temporary effects. */
/**
 * @brief Create a frame-0 sprite at fixed-point x/y, one layer above the parent, with a 120-tick counter.
 */
void Zombie_SpawnTemporarySprite(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, ZombieTemporarySprite_Update);
    if (part)
    {
        s32 zero;
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer + 1;
        zero = 0;
        graphics = ObjGfx_Find(sZombieGfx);
        Sprite_Attach(part, sZombieSpriteSheet, graphics, parent->basePalette);
        part->animFrame = zero;
        part->ext.zombieSprite.lifetime = 120;
        part->scrollWithBg = 1;
        part->drawFunc = Sprite_DrawEntity;
    }
}

/* Zombie debris and temporary effects. */
/**
 * @brief Delete the temporary sprite when its 32-bit lifetime counter expires.
 */
void ZombieTemporarySprite_Update(struct EwramData_EntityData *entity)
{
    if (entity->ext.zombieSprite.lifetime-- <= 0) EntityDelete(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ZombiePiece_Update = ZombieSoldierPiece_Update
 *   Zombie_LoadGraphics = LegionWalkingCorpse_LoadGraphics
 *   Zombie_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 7B3E4 | Same as ZombieSoldierPiece_Update.
 */
void ZombiePiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->posY.whole > entity->ext.zombiePiece.floorY)
        EntityDelete(entity);
}

/**
 * @brief 7B404 | Same as LegionWalkingCorpse_LoadGraphics.
 */
u8 Zombie_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sZombieGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sZombiePalette, 0);
    Sprite_Attach(entity, sZombieSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7B450 | Same as Enemy_DamagePlayerOnContact.
 */
void Zombie_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
