/*
 * Une: body, growth and debris.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_hit.h"

void Une_UpdateGrow(struct EwramData_EntityData *entity);
void Une_UpdateIdle(struct EwramData_EntityData *entity);
u8 Une_LoadGraphics(struct EwramData_EntityData *entity);
void Une_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Une_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void Une_SpawnDebris(s32, s32, s32, s32, s32);
void UneDebris_Update(struct EwramData_EntityData *);

/**
 * @brief 74130 | Une update: unless state 3 (waiting for its parent), the
 * on-screen test (except while dying), hidden off screen (hidden) and
 * Entity_IsOutsideRoomMap starting the dying state 2; states 0 / 1 (Une_UpdateGrow /
 * Une_UpdateIdle), 2 blinks one frame then sets deleteRequested and decrements the
 * count of the parent (enemyId 0x3D, counter unk_4FC), 3 finishes once its
 * own counter unk_4FC is zero, 4 animation only.
 */
void EnemyUneUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    struct EwramData_EntityData *parent;

    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
            entity->hidden = 1;
        else
            entity->hidden = 0;
        if (Entity_IsOutsideRoomMap(entity))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(&result, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Une_UpdateGrow(entity);
        break;
    case 1:
        Une_UpdateIdle(entity);
        break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        if (entity->deleteRequested)
        {
            parent = entity->ext.une.spawner;
            if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x3D
                && parent->ext.uneSpawner.count != 0)
                parent->ext.uneSpawner.count--;
        }
        break;
    case 3:
        if (entity->ext.uneSpawner.count == 0)
            entity->deleteRequested = 1;
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/**
 * @brief 7427C | Une create: deleted when Une_LoadGraphics fails; frozen time
 * (bestiaryPreview) gives state 4 with animation 3; otherwise Entity_EnableBoxes
 * callbacks, and when spawnParam1 is set (also after Entity_SnapToGround) it becomes
 * the waiting state 3 hidden and without boxes, creating a child Une of
 * the same template that it counts in unk_4FC (child unk_500 = 1); else
 * state 0 with timer 60. accelY = 0x2800.
 */
void EnemyUneCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    if ((u8)Une_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        Anim_StartIndexed(entity, 3, 0, 1);
        Anim_Update(entity);
        return;
    }
    Entity_EnableBoxes(entity, Une_DamagePlayerOnContact, Une_OnHit);
    if (entity->spawnParam1 != 0)
    {
        Entity_SnapToGround(entity);
        if (entity->spawnParam1 != 0)
        {
            entity->state = 3;
            entity->hidden = 1;
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->scrollWithBg = 1;
                child->spawnIndex = 0;
                child->enemyId = entity->enemyId;
                child->spawnParam0 = 0;
                child->spawnParam1 = 0;
                child->drawFunc = Sprite_DrawEntity;
                child->hp = sEnemyData[entity->enemyId].hp;
                child->mp = sEnemyData[entity->enemyId].mp;
                child->updateFunc = entity->updateFunc;
                EnemyUneCreate(child);
                child->accelY.whole = 0x2800;
                child->ext.une.spawner = entity;
                entity->ext.uneSpawner.count = 1;
                child->ext.une.original = 1;
            }
            entity->accelY.whole = 0x2800;
            return;
        }
    }
    entity->state = 0;
    entity->timer = 0x3C;
    entity->accelY.whole = 0x2800;
}

/**
 * @brief 743B4 | Une spawner AI: falls/bounces until it lands (Entity_MoveWithPointCollision),
 * wakes on the player box 0xC8 x 0x140 (state 10 = spawned child, straight
 * to state 1), opens (animations 1-3) and waits; when the timer unk_504
 * runs out it checks the parent enemy 0x3D's child count (unk_4FC vs
 * spawnParam0) and either keeps spawning (state 4: a child Une every 30 frames
 * via EnemyUneCreate, with debris Une_SpawnDebris) or closes (animation 5,
 * then state = 2).
 */
void Une_UpdateGrow(struct EwramData_EntityData *entity)
{
    s32 res;
    s32 speed;
    struct EwramData_EntityData *parent;
    struct EwramData_EntityData *child;
    u8 t;
    s32 vx;
    s32 vy;

    switch (entity->subState)
    {
    case 0:
    case 10:
        if (entity->accelY.whole)
        {
            speed = 0x10000;
            res = Entity_MoveWithPointCollision(entity, speed, -0x80000);
            if (res & 3)
                entity->velX.whole = -entity->velX.whole;
            if (res & 8)
                entity->velY.whole += speed;
        }
        else
        {
            res = 0xF;
        }
        if (!(res & 4))
            break;
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0;
        if (entity->subState == 10)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->animDone = 0;
        }
        else if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x64), (s16)(entity->posY.part16.integer - 0xA0), 0xC8, 0x140))
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState++;
            entity->animDone = 0;
        }
        entity->ext.une.spreadTimer = RandomNumberGenerator() % 200 + 0x118;
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState++;
            entity->animDone = 0;
        }
        break;
    case 2:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->subState++;
            entity->animDone = 0;
            entity->timer = RandomNumberGenerator() % 40 + 100;
            if (entity->ext.une.original)
                entity->timer = 10;
        }
        break;
    case 3:
        t = entity->timer--;
        if (t == 0)
        {
            parent = entity->ext.une.spawner;
            if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x3D
                && (s32)parent->ext.uneSpawner.count < parent->spawnParam0)
            {
                entity->timer = t;
                entity->subState++;
                break;
            }
            entity->timer = RandomNumberGenerator() % 40 + 100;
        }
        if (--entity->ext.une.spreadTimer > 0)
            break;
        parent = entity->ext.une.spawner;
        if (parent == NULL || parent->updateFunc == NULL || parent->enemyId != 0x3D)
            break;
        if ((s32)parent->ext.uneSpawner.count >= parent->spawnParam0)
        {
            entity->subState = 5;
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->animDone = 0;
        }
        else
        {
            entity->ext.une.spreadTimer = 0x8C;
        }
        break;
    case 4:
        if (entity->timer++ > 0x1D)
        {
            entity->timer = RandomNumberGenerator() % 40 + 100;
            entity->subState = 3;
        }
        if (entity->timer == 0xF)
        {
            parent = entity->ext.une.spawner;
            child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole - 0x10000;
                child->scrollWithBg = 1;
                child->spawnIndex = 0;
                child->enemyId = entity->enemyId;
                child->spawnParam0 = 0;
                child->spawnParam1 = 0;
                child->drawFunc = Sprite_DrawEntity;
                child->hp = sEnemyData[entity->enemyId].hp;
                child->mp = sEnemyData[entity->enemyId].mp;
                child->updateFunc = entity->updateFunc;
                EnemyUneCreate(child);
                child->velX.whole = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
                child->velY.whole = -0x30000 - (RandomNumberGenerator() & 0x1FFFF);
                child->accelX.whole = 0;
                child->accelY.whole = 0x2800;
                child->subState = 10;
                child->ext.une.spawner = parent;
                parent->ext.uneSpawner.count++;
                child->ext.une.original = 0;
            }
            else
            {
                parent = entity->ext.une.spawner;
                if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x3D)
                    parent->ext.uneSpawner.count--;
            }
        }
        if (!(entity->timer & 3))
        {
            vx = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
            vy = -0x20000 - (RandomNumberGenerator() & 0x1FFFF);
            Une_SpawnDebris(entity->posX.whole, entity->posY.whole, vx, vy, entity->spriteLayer - 1);
        }
        break;
    case 5:
        if (entity->animDone)
        {
            entity->state = 2;
            entity->timer = 1;
        }
        break;
    }
}

void Une_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief Spawn a debris particle (animation 4) at (x, y) moving with
 * (velocityX, velocityY) on layer.
 */
void Une_SpawnDebris(s32 x, s32 y, s32 velocityX, s32 velocityY, s32 layer)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, UneDebris_Update);
    if (effect != NULL)
    {
        s32 slot;
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->velX.whole = velocityX;
        effect->velY.whole = velocityY;
        effect->timer = 0;
        effect->spriteLayer = layer;
        slot = ObjGfx_Find(sUneGfx);
        Sprite_Attach(effect, sUneSpriteSheet, slot, 6);
        Anim_StartIndexed(effect, 4, 0, 1);
        Anim_Update(effect);
        effect->scrollWithBg = 1;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Slow and animate a particle, removing it after its timer passes 20. */

/**
 * @brief Debris update: slow down, fall and delete after 20 frames.
 */
void UneDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 8);
    EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    Anim_Update(entity);
    if (entity->timer++ > 0x13)
        EntityDelete(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Une_LoadGraphics = Object25_LoadGraphics */

/**
 * @brief 74874 | Same as Object25_LoadGraphics.
 */
u8 Une_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sUneGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnePalette, 0);
    Sprite_Attach(entity, sUneSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Une_UpdateIdle. */
/**
 * @brief 748C0 | Sub-state 0: start the animation (Anim_StartIndexed with
 * 0, 0, 0) and advance the sub-state.
 */
void Une_UpdateIdle(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->subState++;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Une_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 748E4 | Same as Enemy_DamagePlayerOnContact.
 */
void Une_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
