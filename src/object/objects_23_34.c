/*
 * Room objects 0x23-0x34.
 */
#include "core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "player_core.h"
#include "sound.h"
#include <string.h>
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "objects_23_34.h"
#include "palette.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "obj_palette.h"
#include "entity_hit.h"

/* Target_IsInRect (engine/entity_common.c) takes u16 there; the callers here pass
 * sign-extended values. */
u8 Object26_LoadGraphics(struct EwramData_EntityData *entity);
void Object26_UpdateState(struct EwramData_EntityData *entity);

u8 Object23_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object24_LoadGraphics(struct EwramData_EntityData *entity);
void Object25_UpdateState(struct EwramData_EntityData *entity);
u8 Object25_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object28_LoadGraphics(struct EwramData_EntityData *entity);
void Object28_UpdateState(struct EwramData_EntityData *entity);
void Object28_OnPlayerContact(struct EwramData_EntityData *entity);
void Object27_UpdateState(struct EwramData_EntityData *entity);
void Object27_OnPlayerContact(struct EwramData_EntityData *entity);
u8 Object27_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object2B_LoadGraphics(struct EwramData_EntityData *entity);
void Object2CLayer_Update(struct EwramData_EntityData *entity);
void Object2C_SetupLayer(struct EwramData_EntityData *entity);
u8 Object2C_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object2D_LoadGraphics(struct EwramData_EntityData *entity);
u8 Object2E_LoadGraphics(struct EwramData_EntityData *entity);
void Object2E_UpdateState(struct EwramData_EntityData *entity);
void Object2EPiece_Update(struct EwramData_EntityData *entity);
void Object2E_SpawnRisingParticle(struct EwramData_EntityData *entity, s32 x, s32 y);
u8 Object2F_LoadGraphics(struct EwramData_EntityData *entity);
void Object2F_UpdateState(struct EwramData_EntityData *entity);
void FallingLeaf_Update(struct EwramData_EntityData *entity);
u8 LeafSpawner_LoadGraphics(struct EwramData_EntityData *entity);
void LeafSpawner_UpdateState(struct EwramData_EntityData *entity);
u8 Object31_LoadGraphics(struct EwramData_EntityData *entity);
void Object31_UpdateState(struct EwramData_EntityData *entity);
void Object31_SpawnDust(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);
u8 CrumblingPlatform_LoadGraphics(struct EwramData_EntityData *entity);
void CrumblingPlatform_UpdateState(struct EwramData_EntityData *entity);
void CrumblingPlatform_SpawnDebris(struct EwramData_EntityData *entity, s32 frame, s32 index);
void CrumblingPlatformDebris_Update(struct EwramData_EntityData *entity);
u8 SparkEmitter_LoadGraphics(struct EwramData_EntityData *entity);
void SparkEmitter_SpawnFlyingSpark(struct EwramData_EntityData *entity);
void SparkEmitter_SpawnFallingSpark(struct EwramData_EntityData *entity);
void SparkEmitterFallingSpark_Update(struct EwramData_EntityData *entity);
void SparkEmitter_UpdateState(struct EwramData_EntityData *entity);
void SparkEmitterFlyingSpark_Update(struct EwramData_EntityData *entity);
u8 PendulumPlatform_LoadGraphics(struct EwramData_EntityData *entity);
void PendulumPlatform_SpawnPart(struct EwramData_EntityData *entity, s32 part);
void PendulumPlatformPart_Update(struct EwramData_EntityData *entity);
void PendulumPlatformPart_Draw(struct EwramData_EntityData *entity);

/* Objects 0x23 and 0x24 0x080577FC..0x080579A3. */

/**
 * @brief 577FC | Object 0x23 update: animate.
 */
void Object23Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    Anim_Update(entity);
}

/**
 * @brief 5780C | Object 0x23 create: load the graphics (delete the object
 * when that fails), set animation 1 and layer 0x14.
 */
void Object23Create(struct EwramData_EntityData *entity)
{
    if (!Object23_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x14;
    entity->objMode = 1;
    entity->state = 0;
}

/**
 * @brief 57858 | Load the graphics of object 0x23; 0 when no slot is free.
 */
u8 Object23_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sCrusherObject23Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 3);
    Sprite_Attach(entity, sCrusherObject23SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 578A4 | Object 0x24 create: load the graphics (delete the object
 * when that fails), set animation 2 with random animTimer (0..15) and
 * animStep (0..2), layer 0x13.
 */
void Object24Create(struct EwramData_EntityData *entity)
{
    if (!Object24_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 2, 0, 1);
    entity->animTimer = RandomNumberGenerator() % 16;
    entity->animStep = RandomNumberGenerator() % 3;
    Anim_Update(entity);
    entity->spriteLayer = 0x13;
    entity->objMode = 1;
    entity->state = 0;
}

/**
 * @brief 5790C | Object 0x24 update: animate on 7 of 8 frames at random and
 * toggle 0x53C bit 5 every frame (flicker).
 */
void Object24Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if ((RandomNumberGenerator() & 0xFF) < 0xE0)
        Anim_Update(entity);
    if (entity->timer++ & 1)
        entity->hidden = 1;
    else
        entity->hidden = 0;
}

/**
 * @brief 57958 | Load the graphics of object 0x24; 0 when no slot is free.
 */
u8 Object24_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject24Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 7);
    Sprite_Attach(entity, sObject24SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Object 0x25 (Object25Update / Object25Create). */

/* Per variant (spawnParam0): spawn point list and its length. */

/**
 * @brief 579A4 | Object 0x25 update: state 0 -> Object25_UpdateState; state 1 is a
 * spawner that every 0x78 frames (50 %) creates up to 10 copies of itself
 * at a random point of its variant's list (screen relative to BG 1).
 */
void Object25Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *child;
    const struct Object25Point *points;
    u32 randomValue;
    u32 pointIndex;
    u32 collision[4];

    switch (entity->state)
    {
        case 0:
            Object25_UpdateState(entity);
            break;
        case 1:
            if (entity->timer++ < 0x78)
                break;
            entity->timer = 0;
            randomValue = RandomNumberGenerator() % 2;
            if (randomValue != 0)
                break;
            if ((s32)entity->ext.object25Spawner.childCount > 9)
                break;
            child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
            if (child == NULL)
                break;
            entity->ext.object25Spawner.childCount++;
            points = sObject25Points[entity->spawnParam0];
            pointIndex = sObject25PointIndices[entity->spawnParam0];
            pointIndex = RandomNumberGenerator() % pointIndex;
            child->posX.whole = points[pointIndex].x - bg->xPos.whole;
            child->posY.whole = points[pointIndex].y - bg->yPos.whole;
            child->scrollWithBg = 1;
            child->spawnIndex = randomValue;
            child->enemyId = entity->enemyId;
            child->spawnParam1 = randomValue;
            child->spawnParam0 = entity->spawnParam0;
            child->drawFunc = Sprite_DrawEntity;
            child->updateFunc = entity->updateFunc;
            Object25Create(child);
            child->ext.object25.spawner = entity;
            break;
    }
}

/**
 * @brief 57AA0 | Object 0x25 create. Variant spawnParam1 == 1 becomes the
 * spawner (state 1); otherwise load graphics and set up; spawnParam1 == 2
 * starts moving at once. Deleted when bossRushMode == 0 and bossEventFlags bit 5.
 */
void Object25Create(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.bossRushMode == 0 && (gEwramData->gameState.bossEventFlags & 0x20))
    {
        EntityDelete(entity);
        return;
    }
    if (entity->spawnParam1 == 1)
    {
        entity->state = 1;
        entity->hidden = 1;
        entity->timer = 0x78;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Entity_SnapToGround(entity);
        return;
    }
    if (!Object25_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_Start(entity, (struct AnimScript *)sObject25AnimScript0, 0, 0);
    Anim_Update(entity);
    entity->enemyFlags |= 1;
    entity->attack = 0x28;
    entity->hp = 0xC8;
    entity->spriteLayer = 0x14;
    Entity_SnapToGround(entity);
    entity->state = 0;
    if (entity->spawnParam1 == 2)
    {
        entity->subState = 1;
        Anim_Start(entity, (struct AnimScript *)sObject25AnimScript1, 0, 1);
        Anim_Update(entity);
        entity->velX.whole = 0x4000;
        entity->velY.whole = 0x40000;
        entity->spriteLayer--;
    }
}

/* Objects 0x25..0x28 0x08057BAC..0x08058507. */

/* Animations of object 0x25 (Anim_Start). */

/**
 * @brief 57BAC | Object 0x25, state 0: wait for 0x53D bit 0, then walk
 * right (falling, Entity_MoveWithCollisionDefault) until the variant's end x (world, BG 1);
 * substate 10 blinks with Effect_SpawnRandom effects and finally ends the object,
 * decrementing the spawner's counter (unk_500 -> unk_4FC).
 */
void Object25_UpdateState(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *spawner = entity->ext.object25.spawner;
    struct HitResult result;

    switch (entity->subState)
    {
        case 0:
            if (!entity->animDone)
                break;
            entity->subState = 1;
            Anim_Start(entity, (struct AnimScript *)sObject25AnimScript1, 0, 1);
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0x40000;
            entity->spriteLayer--;
            break;
        case 1:
            entity->velX.whole = 0x4000;
            EntityMoveX(entity);
            entity->velX.whole = 0;
            if (entity->spawnParam0 != 2)
            {
                entity->velY.whole = 0x40000;
                Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
            }
            else if (entity->posX.whole + bg->xPos.whole <= 0xD00000)
            {
                entity->velY.whole = 0x40000;
                Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
            }
            Enemy_UpdateHitFlash(entity);
            switch (entity->spawnParam0)
            {
                case 0:
                case 1:
                    if (entity->posX.whole + bg->xPos.whole > 0x40FFFFF)
                    {
                        entity->subState = 10;
                        entity->timer = 1;
                    }
                    break;
                case 2:
                case 3:
                    if (entity->posX.whole + bg->xPos.whole > 0xFFFFFF)
                    {
                        entity->subState = 10;
                        entity->timer = 1;
                    }
                    break;
            }
            break;
        case 10:
            if ((entity->timer & 3) == 0)
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x60000, 0x100000, 0xC0000, entity->spriteLayer - 1);
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)--entity->timer > 0)
                break;
            entity->deleteRequested = 1;
            if (spawner != NULL && spawner->updateFunc != NULL && spawner->ext.object25Spawner.childCount != 0)
                spawner->ext.object25Spawner.childCount--;
            return;
    }
    Anim_Update(entity);
    if (entity->subState == 1)
        Hit_CheckWithPlayer(&result, entity);
}

/**
 * @brief 57D38 | Object 0x25 hit: apply the damage of hit to the HP in
 * hp; at 0 switch to substate 10 (dying) with animation sObject25AnimScript2.
 */
void Object25_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    // (s16)damage, written so that agbcc sign-extends in place
    damage <<= 16;
    damage >>= 16;
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hp -= damage;
    if ((s16)entity->hp <= 0)
    {
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->hitFlashTimer = 0;
        entity->subState = 10;
        Anim_Start(entity, (struct AnimScript *)sObject25AnimScript2, 2, 1);
    }
}

/**
 * @brief 57DB8 | Load the graphics of object 0x25; 0 when no slot is free.
 * The same code exists 28 times in the ROM with other data.
 */
u8 Object25_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject25Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sLegionObject25Palette, 0);
    Sprite_Attach(entity, sObject25SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 57E04 | Object 0x26 create: load the graphics (delete the object
 * when that fails), set up and spawn objects 0x28 (-0x15, +0x44) and 0x27
 * (+0x12, +0x2C) linked back through their unk_4F8.
 */
void Object26Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    if (!Object26_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 3, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->flipX = 0;
    entity->scrollWithBg = 1;
    entity->hidden = 1;
    entity->ext.object26.clock28 = 0x5A;
    entity->ext.object26.clock27 = 0x1E;
    entity->state = 0;
    child = EntityCreateInRange(0x1B, 0x48, Object28Update);
    if (child != NULL)
    {
        child->posX.part16.integer = entity->posX.part16.integer - 0x15;
        child->posY.part16.integer = entity->posY.part16.integer + 0x44;
        Object28Create(child);
        ENTITY_OWNER(child) = entity;
    }
    child = EntityCreateInRange(0x1B, 0x48, Object27Update);
    if (child != NULL)
    {
        child->posX.part16.integer = entity->posX.part16.integer + 0x12;
        child->posY.part16.integer = entity->posY.part16.integer + 0x2C;
        Object27Create(child);
        ENTITY_OWNER(child) = entity;
    }
}

/**
 * @brief 57EE0 | Object 0x26 update.
 */
void Object26Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object26_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 57F00 | Load the graphics of object 0x26; 0 when no slot is free.
 */
u8 Object26_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject13FamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject13FamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 57F4C | Object 0x26, state 0: while Target_IsInRect sees the area
 * (x-0x46, y-0x64, 0x8C x 0xC8) count up unk_4FC / unk_4FE; 0x53C bit 5
 * is set either way.
 */
void Object26_UpdateState(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if (Target_IsInRect((s16)(entity->posX.part16.integer - 0x46), (s16)(entity->posY.part16.integer - 0x64), 0x8C, 0xC8))
    {
        entity->hidden = 1;
        entity->ext.object26.clock27++;
        entity->ext.object26.clock28++;
    }
    else
    {
        entity->hidden = 1;
    }
}

/**
 * @brief 57FA8 | Object 0x28 create: load the graphics, register the hit
 * box (Object28_OnPlayerContact on contact), damage from the player's stats, allocate
 * an affine slot (delete the object when anything fails).
 */
void Object28Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 affineIndex;
    u16 attack;

    if (!Object28_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, Object28_OnPlayerContact, 0);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    // Same contact damage formula as in Player_ApplyContactDamage.
    attack = ((*ewramRef)->playerState.equipmentDefenseBonus >> 1) + ((*ewramRef)->playerState.equipmentDefenseBonus >> 2)
        + ((*ewramRef)->playerStatus.maxHP >> 4);
    entity->attack = attack;
    entity->spriteLayer = 0x12;
    entity->flipX = 0;
    entity->scrollWithBg = 1;
    entity->hidden = 1;
    affineIndex = ObjAffine_Alloc(1);
    if ((s16)affineIndex < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    ObjAffine_SetBiased(0, 0, 0, entity->flipX, entity->flipY, entity->affineIndex);
    entity->state = 0;
}

/**
 * @brief 58094 | Object 0x28, state 0: once the parent (unk_4F8) counted
 * 0x78 frames in unk_4FE, appear and grow (affine scale in velX) from
 * 0xC000 to 0x10000, stay with an active hit box for 30 frames, shrink back
 * and hide again.
 */
void Object28_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *controller = ENTITY_OWNER(entity);
    s32 zero = 0;
    u32 hitBox = 0x1010FCF8;

    switch (entity->subState)
    {
        case 0:
            if (controller->ext.object26.clock28 < 0x78)
                break;
            controller->ext.object26.clock28 = zero;
            entity->subState++;
            entity->hidden = 0;
            entity->velX.whole = 0xC000;
            break;
        case 1:
            if (entity->velX.whole < 0x10000)
            {
                entity->velX.whole += 0x800;
            }
            else
            {
                entity->velX.whole = 0x10000;
                entity->subState++;
                entity->timer = 0x1E;
                HitBox_SetAttack(entity, &hitBox);
                entity->hit.flags &= ~2;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
        case 2:
            if (entity->timer-- > 1)
                break;
            entity->subState++;
            entity->hit.flags |= 2;
            break;
        case 3:
            if (entity->velX.whole > 0xC000)
            {
                entity->velX.whole -= 0x800;
            }
            else
            {
                entity->subState = zero;
                entity->hidden = 1;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
    }
}

/**
 * @brief 581B4 | Object 0x28 update.
 */
void Object28Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 0)
        Object28_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(&result, entity);
}

/**
 * @brief 581DC | Same as Object26_LoadGraphics.
 */
u8 Object28_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject13FamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject13FamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 58228 | Contact callback: Player_ApplyContactDamage with (attack, 0, 0, 1) at
 * the object position.
 */
void Object28_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 58258 | Object 0x27 create: same as Object28Create with animation 2.
 */
void Object27Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 affineIndex;
    u16 attack;

    if (!Object27_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, Object27_OnPlayerContact, 0);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 2, 0, 1);
    Anim_Update(entity);
    // Same contact damage formula as in Player_ApplyContactDamage.
    attack = ((*ewramRef)->playerState.equipmentDefenseBonus >> 1) + ((*ewramRef)->playerState.equipmentDefenseBonus >> 2)
        + ((*ewramRef)->playerStatus.maxHP >> 4);
    entity->attack = attack;
    entity->spriteLayer = 0x12;
    entity->flipX = 0;
    entity->scrollWithBg = 1;
    entity->hidden = 1;
    affineIndex = ObjAffine_Alloc(1);
    if ((s16)affineIndex < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    ObjAffine_SetBiased(0, 0, 0, entity->flipX, entity->flipY, entity->affineIndex);
    entity->state = 0;
}

/**
 * @brief 58344 | Object 0x27, state 0: same as Object28_UpdateState with the parent
 * counter unk_4FC and a bigger hit box.
 */
void Object27_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *controller = ENTITY_OWNER(entity);
    s32 zero = 0;
    u32 hitBox = 0x2010F0F8;

    switch (entity->subState)
    {
        case 0:
            if (controller->ext.object26.clock27 < 0x78)
                break;
            controller->ext.object26.clock27 = zero;
            entity->subState++;
            entity->hidden = 0;
            entity->velX.whole = 0xC000;
            break;
        case 1:
            if (entity->velX.whole < 0x10000)
            {
                entity->velX.whole += 0x800;
            }
            else
            {
                entity->velX.whole = 0x10000;
                entity->subState++;
                entity->timer = 0x1E;
                HitBox_SetAttack(entity, &hitBox);
                entity->hit.flags &= ~2;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
        case 2:
            if (entity->timer-- > 1)
                break;
            entity->subState++;
            entity->hit.flags |= 2;
            break;
        case 3:
            if (entity->velX.whole > 0xC000)
            {
                entity->velX.whole -= 0x800;
            }
            else
            {
                entity->subState = zero;
                entity->hidden = 1;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
    }
}

/**
 * @brief 58464 | Object 0x27 update.
 */
void Object27Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (entity->state == 0)
        Object27_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(&result, entity);
}

/**
 * @brief 5848C | Same as Object26_LoadGraphics.
 */
u8 Object27_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject13FamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject13FamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 584D8 | Same as Object28_OnPlayerContact.
 */
void Object27_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Objects 0x2B and 0x2C 0x08058508..0x0805872B. */

/**
 * @brief 58508 | Object 0x2B create: load the graphics (delete the object
 * when that fails), animation 1, layer 0x12.
 */
void Object2BCreate(struct EwramData_EntityData *entity)
{
    if (!Object2B_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->state = 0;
}

/**
 * @brief 58544 | Object 0x2B update: animate.
 */
void Object2BUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    Anim_Update(entity);
}

/**
 * @brief 58554 | Load the graphics of object 0x2B (with palette data
 * sObject2BPaletteAnim); 0 when no slot is free.
 */
u8 Object2B_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2BGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 1);
    PaletteAnim_Start(sUnk_0820A1C4, sObject2BPaletteAnim, entity->basePalette);
    Sprite_Attach(entity, sObject2BSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 585B0 | Object 0x2C create: spawn four Object2CLayer_Update layers at a
 * random x, y 0x1E/0x3E/0x56/0x66, moving right with speeds 0x4000..0x1000.
 */
void Object2CCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 i;
    s32 zero;

    for (i = 0, zero = 0; i <= 3; i++)
    {
        child = EntityCreateInRange(0x49, 0xCF, Object2CLayer_Update);
        if (child == NULL)
            continue;
        child->spawnIndex = zero;
        child->enemyId = entity->enemyId;
        child->drawFunc = Sprite_DrawEntity;
        Object2C_SetupLayer(child);
        child->posX.whole = RandomNumberGenerator() & 0xFFFFFF;
        switch (i)
        {
            case 0:
                child->posY.part16.integer = 0x1E;
                child->velX.whole = 0x4000;
                child->animFrame = zero;
                break;
            case 1:
                child->posY.part16.integer = 0x3E;
                child->velX.whole = 0x3000;
                child->animFrame = i;
                break;
            case 2:
                child->posY.part16.integer = 0x56;
                child->velX.whole = 0x2000;
                child->animFrame = i;
                break;
            case 3:
                child->posY.part16.integer = 0x66;
                child->velX.whole = 0x1000;
                child->animFrame = i;
                break;
        }
    }
}

/**
 * @brief 58664 | Object 0x2C update: nothing.
 */
void Object2CUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];
}

/**
 * @brief 5866C | Object 0x2C layer: move right and wrap to x -0x3C after
 * leaving the screen.
 */
void Object2CLayer_Update(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        EntityMoveX(entity);
        if (EntityIsOutsideScreen(entity, 6))
            entity->posX.part16.integer = -0x3C;
    }
}

/**
 * @brief 5869C | Set up an object 0x2C layer: load the graphics (delete it
 * when that fails), layer 0x1D.
 */
void Object2C_SetupLayer(struct EwramData_EntityData *entity)
{
    if (!Object2C_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->spriteLayer = 0x1D;
    entity->state = 0;
    entity->scrollWithBg = 0;
}

/**
 * @brief 586E0 | Load the graphics of object 0x2C; 0 when no slot is free.
 */
u8 Object2C_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2CGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 0);
    Sprite_Attach(entity, sObject2CSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/* Objects 0x2D..0x31 0x0805872C..0x08059493. */
#define PLAYER_ENTITY (gEwramData->playerState.player)

/* Target_IsInRect (engine/entity_common.c) takes u16 there; the callers here pass
 * sign-extended values. */

/**
 * @brief 5872C | Object 0x2D, state 0: substate 10 = idle animations 0 -> 1
 * -> 2 / 3, 4 -> 0 (each on animation end, 0x53D bit 0); go to substate 12
 * when the player is within 0x50 x 0x60 px, where animations 3, 4 -> 0,
 * 2 -> 4, 1 -> 3, and back to 10 when the player leaves. Hidden while
 * outside the screen.
 */
void Object2D_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;
    s32 dx = player->posX.whole - entity->posX.whole;
    s32 dy = player->posY.whole - entity->posY.whole;
    s32 dist;

    switch (entity->subState)
    {
        case 10:
            if (entity->animScriptIndex == 0 && entity->animDone)
                Anim_StartIndexed(entity, 1, 0, 0);
            if (entity->animScriptIndex == 1 && entity->animDone)
                Anim_StartIndexed(entity, 2, 0, 0);
            if (entity->animScriptIndex == 3 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if (entity->animScriptIndex == 4 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if ((dx < 0 ? -dx : dx) > 0x4FFFFF)
                break;
            if ((dy < 0 ? -dy : dy) > 0x5FFFFF)
                break;
            entity->subState = 12;
            break;
        case 12:
            if (entity->animScriptIndex == 3 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if (entity->animScriptIndex == 4 && entity->animDone)
                Anim_StartIndexed(entity, 0, 0, 0);
            if (entity->animScriptIndex == 2 && entity->animDone)
                Anim_StartIndexed(entity, 4, 0, 0);
            if (entity->animScriptIndex == 1 && entity->animDone)
                Anim_StartIndexed(entity, 3, 0, 0);
            dist = dx;
            if (dist < 0)
                dist = -dist;
            if (dist > 0x4FFFFF)
            {
                entity->subState = 10;
                return;
            }
            dist = dy;
            if (dist < 0)
                dist = -dist;
            if (dist > 0x5FFFFF)
            {
                entity->subState = 10;
                return;
            }
            break;
    }
    if (EntityIsOutsideScreen(entity, 3))
    {
        entity->hidden = 1;
    }
    else
    {
        entity->hidden = 0;
        Entity_HideAboveY(entity, -0x18);
    }
}

/**
 * @brief 588EC | Object 0x2D create: load the graphics (delete the object
 * when that fails), animation 2, start in substate 10.
 */
void Object2DCreate(struct EwramData_EntityData *entity)
{
    if (!Object2D_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 2, 0, 0);
    entity->animTimer = 0;
    entity->animStep = 6;
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->subState = 10;
}

/**
 * @brief 58938 | Object 0x2D update.
 */
void Object2DUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object2D_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 58958 | Load the graphics of object 0x2D; 0 when no slot is free.
 */
u8 Object2D_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2DGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject2DObject2EPalette, 2);
    Sprite_Attach(entity, sObject2DSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 589A4 | Spawn a Object2EPiece_Update piece of object 0x2E at offset
 * (offsetX, offsetY) with random frame and facing, plus a Object2E_SpawnRisingParticle
 * effect 8 px below it.
 */
void Object2E_SpawnPiece(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Object2EPiece_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + offsetX;
    child->posY.whole = entity->posY.whole + offsetY;
    child->spriteLayer = entity->spriteLayer;
    gfx = ObjGfx_Find(sObject2EGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject2ESpriteSheet, gfx, child->basePalette);
    Anim_StartIndexed(child, 0, 0, 1);
    Anim_Update(child);
    child->animTimer = RandomNumberGenerator() % 8;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
    Object2E_SpawnRisingParticle(child, child->posX.whole, child->posY.whole + 0x80000);
}

/**
 * @brief 58A70 | Object 0x2E create: load the graphics (delete the object
 * when that fails), hidden (0x53C bit 5) until triggered.
 */
void Object2ECreate(struct EwramData_EntityData *entity)
{
    if (!Object2E_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->hidden = 1;
}

/**
 * @brief 58AB8 | Object 0x2E update.
 */
void Object2EUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object2E_UpdateState(entity);
}

/**
 * @brief 58AD0 | Load the graphics of object 0x2E; 0 when no slot is free.
 */
u8 Object2E_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2EGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject2DObject2EPalette, 5);
    Sprite_Attach(entity, sObject2ESpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 58B1C | Object 0x2E, state 0: when the area (x-0x20, y-0x20,
 * 0x40 x 0x70) is entered, spawn three pieces (left, above, right) and
 * play sound 0x78.
 */
void Object2E_UpdateState(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0
        && Target_IsInRect((s16)(entity->posX.part16.integer - 0x20), (s16)(entity->posY.part16.integer - 0x20), 0x40, 0x70))
    {
        entity->subState++;
        Object2E_SpawnPiece(entity, -0x50000, 0);
        Object2E_SpawnPiece(entity, 0, -0x60000);
        Object2E_SpawnPiece(entity, 0x50000, 0);
        PlaySong(0x78);
    }
}

/**
 * @brief 58B84 | Object 0x2E piece update: animate.
 */
void Object2EPiece_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    Anim_Update(entity);
}

/**
 * @brief 58B94 | Spawn a rising Effect_UpdateParticle effect at (x, y)
 * one layer below the entity.
 */
void Object2E_SpawnRisingParticle(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velY.whole = -0x10000;
    child->spriteLayer = entity->spriteLayer - 1;
    child->timer = 0;
    child->ext.particle.flags = 2;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    Anim_Start(child, (struct AnimScript *)sObject2EAnimScript, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief 58C1C | Object 0x2F create: deleted when bossEventFlags bit 2 is set or
 * the graphics fail; random frame, start position by variant (spawnParam0).
 */
void Object2FCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 xs[3];
    u16 ys[3];

    memcpy(xs, sObject2FXs, sizeof(xs));
    memcpy(ys, sObject2FYs, sizeof(ys));
    if (((*ewramRef)->gameState.bossEventFlags & 4) || !Object2F_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 6, 0, 1);
    Anim_Update(entity);
    entity->animTimer = RandomNumberGenerator() % 6;
    entity->spriteLayer = 0x1A;
    entity->scrollWithBg = 0;
    entity->hidden = 1;
    entity->posX.part16.integer = xs[entity->spawnParam0];
    entity->posY.part16.integer = ys[entity->spawnParam0];
}

/**
 * @brief 58CD8 | Object 0x2F update.
 */
void Object2FUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object2F_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 58CF8 | Load the graphics of object 0x2F; 0 when no slot is free.
 */
u8 Object2F_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2FFamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 9);
    Sprite_Attach(entity, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 58D44 | Object 0x2F, state 0: appear once stateFlags bit 12 is set.
 */
void Object2F_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    if (entity->subState == 0 && (ewram->gameState.stateFlags & 0x1000))
    {
        entity->subState = 1;
        entity->hidden = 0;
    }
}

/**
 * @brief 58D7C | Spawn a falling leaf (FallingLeaf_Update) at a random x at the
 * top of the screen: kind 0..2 (unk_4F8) picks animation, fall speed and
 * parallax; random spin acceleration (accelY) and facing.
 */
void LeafSpawner_SpawnFallingLeaf(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, FallingLeaf_Update);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 gfx;
    u16 affineIndex;

    if (child == NULL)
        return;
    child->posX.part16.integer = RandomNumberGenerator() % 0xF0;
    child->posY.whole = 0;
    child->ext.fallingLeaf.kind = RandomNumberGenerator() % 3;
    child->spriteLayer = entity->spriteLayer;
    gfx = ObjGfx_Find(sObject2FFamilyGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sObject2FFamilySpriteSheet, gfx, child->basePalette);
    Anim_StartIndexed(child, child->ext.fallingLeaf.kind + 2, 0, 1);
    Anim_Update(child);
    affineIndex = ObjAffine_Alloc(1);
    if ((s16)affineIndex < 0)
    {
        EntityDelete(child);
        return;
    }
    child->affineIndex = affineIndex;
    child->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    if (child->ext.fallingLeaf.kind == 0)
        child->velY.whole = 0xA000;
    else if (child->ext.fallingLeaf.kind == 1)
        child->velY.whole = 0x8000;
    else
        child->velY.whole = 0xC000;
    child->accelY.whole = (RandomNumberGenerator() & 0x3FF) - 0x200;
    child->ext.fallingLeaf.scrollX = bg->xPos.whole;
    child->scrollWithBg = 0;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief 58EDC | Falling leaf: fall, spin (affine angle in accelX), follow
 * the BG 1 scroll with parallax 1/4, 1/8 or 3/8 by kind; deleted (with its
 * affine slot) when outside the screen.
 */
void FallingLeaf_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 parallax;

    EntityMoveY(entity);
    entity->accelX.whole += entity->accelY.whole;
    parallax = bg->xPos.whole - (s32)entity->ext.fallingLeaf.scrollX;
    if (entity->ext.fallingLeaf.kind == 0)
        parallax /= 4;
    else if (entity->ext.fallingLeaf.kind == 1)
        parallax /= 8;
    else
        parallax = parallax * 3 / 8;
    entity->posX.whole -= parallax;
    entity->ext.fallingLeaf.scrollX = bg->xPos.whole;
    ObjAffine_Set(entity->accelX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 3))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        Anim_Update(entity);
    }
}

/**
 * @brief 58F88 | Object 0x30 create (leaf spawner): load the graphics
 * (delete the object when that fails), hidden, layer 0x19.
 */
void LeafSpawnerCreate(struct EwramData_EntityData *entity)
{
    if (!LeafSpawner_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x19;
    entity->state = 0;
    entity->hidden = 1;
}

/**
 * @brief 58FD0 | Object 0x30 update.
 */
void LeafSpawnerUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        LeafSpawner_UpdateState(entity);
}

/**
 * @brief 58FE8 | Same as Object2F_LoadGraphics.
 */
u8 LeafSpawner_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2FFamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 9);
    Sprite_Attach(entity, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 59034 | Object 0x30, state 0: once stateFlags bit 12 is set, spawn a
 * leaf every 0x20..0x9F frames.
 */
void LeafSpawner_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;

    switch (entity->subState)
    {
        case 0:
            if (ewram->gameState.stateFlags & 0x1000)
                entity->subState = 1;
            break;
        case 1:
            if (entity->timer-- > 1)
                break;
            entity->timer = (RandomNumberGenerator() % 128) + 0x20;
            LeafSpawner_SpawnFallingLeaf(entity);
            break;
    }
}

/**
 * @brief 59090 | Object 0x31 create: deleted when bossEventFlags bit 2 is set or
 * the graphics fail; position, animation and unk_4F8 by variant (spawnParam0);
 * variants 0 and 2 get an affine slot.
 */
void Object31Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 xs[4];
    u16 ys[4];
    u16 anims[4];
    struct Object31Words values;
    u16 affineIndex;

    memcpy(xs, sObject31Xs, sizeof(xs));
    memcpy(ys, sObject31Ys, sizeof(ys));
    memcpy(anims, sObject31Anims, sizeof(anims));
    values = sUnk_08118674;
    if (((*ewramRef)->gameState.bossEventFlags & 4) || !Object31_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, anims[entity->spawnParam0], 0, 1);
    Anim_Update(entity);
    entity->animTimer = RandomNumberGenerator() % 6;
    entity->spriteLayer = 0x1A;
    entity->scrollWithBg = 0;
    entity->flipX = 0;
    if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
    {
        affineIndex = ObjAffine_Alloc(1);
        if ((s16)affineIndex < 0)
        {
            EntityDelete(entity);
            return;
        }
        entity->affineIndex = affineIndex;
        entity->affine = 1;
        ObjAffine_Set(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
    entity->posX.part16.integer = xs[entity->spawnParam0];
    entity->posY.part16.integer = ys[entity->spawnParam0];
    entity->ext.object31.delay = values.v[entity->spawnParam0];
}

/**
 * @brief 591EC | Object 0x31, state 0: once stateFlags bit 12 is set, wait
 * unk_4F8 frames, then fall (variants 1, 3) or - variants 0, 2 - jump up
 * sideways with a dust effect and fall spinning (affine angle in unk_4FC,
 * spin speed unk_500 up to 0x60); deleted after leaving the screen.
 */
void Object31_UpdateState(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (gEwramData->gameState.stateFlags & 0x1000)
                entity->subState++;
            break;
        case 1:
            if (--entity->ext.object31.delay > 0)
                break;
            if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
            {
                entity->subState = 10;
                entity->ext.object31.angle = 0;
                entity->ext.object31.speed = 0;
                entity->ext.object31.step = 4;
                entity->accelX.whole = 0x400;
                entity->accelY.whole = 0x200;
                entity->timer = 0x3C;
                Object31_SpawnDust(entity, 0, 0xA0000);
            }
            else
            {
                entity->subState++;
                entity->accelY.whole = 0x400;
            }
            break;
        case 2:
            EntityAccelerateAndMoveY(entity, 0x10000);
            if (EntityIsOutsideScreen(entity, 3))
                entity->subState++;
            break;
        case 3:
            if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        case 10:
            EntityAccelerateAndMoveX(entity, 0x4000);
            EntityAccelerateAndMoveY(entity, 0x2000);
            if ((s8)--entity->timer > 0)
                break;
            entity->subState++;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0x400;
            break;
        case 11:
            entity->ext.object31.speed += entity->ext.object31.step;
            if (entity->ext.object31.speed > 0x5F)
                entity->ext.object31.speed = 0x60;
            entity->ext.object31.angle += entity->ext.object31.speed;
            ObjAffine_Set(entity->ext.object31.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            EntityMoveX(entity);
            EntityAccelerateAndMoveY(entity, 0x10000);
            if (EntityIsOutsideScreen(entity, 3))
                entity->subState = 3;
            break;
    }
    if (entity->spawnParam0 == 0 || entity->spawnParam0 == 2)
        ObjAffine_Set(entity->ext.object31.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Update(entity);
}

/**
 * @brief 593A8 | Object 0x31 update.
 */
void Object31Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        Object31_UpdateState(entity);
}

/**
 * @brief 593C0 | Same as Object2F_LoadGraphics.
 */
u8 Object31_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2FFamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_0820A1C4, 9);
    Sprite_Attach(entity, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5940C | Spawn a Effect_UpdateParticle dust effect (animation 5) at offset
 * (offsetX, offsetY), one layer below the entity.
 */
void Object31_SpawnDust(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + offsetX;
    child->posY.whole = entity->posY.whole + offsetY;
    child->spriteLayer = entity->spriteLayer - 1;
    child->timer = 0;
    child->ext.particle.flags = 0;
    gfx = ObjGfx_Find(sObject2FFamilyGfx);
    Sprite_Attach(child, sObject2FFamilySpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 0;
    child->drawFunc = Sprite_DrawEntity;
}

/* Objects 0x32..0x34 0x08059494..0x0805A18B. */

/**
 * @brief 59494 | Object 0x32 create: load the graphics (delete the object
 * when that fails), hit box by spawnParam1, remember the world position
 * (BG 1 relative) in ext.layerObject.x/Y and the shuttle fields (unused by 0x32).
 */
void CrumblingPlatformCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct CrumblingPlatformBoxes boxes = sCrumblingPlatformBoxes;
    s32 roomY;
    s32 roomX;

    if (!CrumblingPlatform_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, NULL, 0);
    HitBox_SetBody(entity, &boxes.box[entity->spawnParam1]);
    HitBox_SetAttack(entity, &boxes.box[entity->spawnParam1]);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    if (entity->spawnParam1 == 0)
        entity->animFrame = 0;
    else
        entity->animFrame = 7;
    entity->spriteLayer = 0x12;
    entity->state = 0;
    roomY = entity->posY.whole;
    entity->ext.layerObject.shuttleStart = roomY + bg->yPos.whole;
    entity->ext.layerObject.shuttleEnd = entity->spawnParam0 << 16;
    roomX = entity->posX.whole + bg->xPos.whole;
    entity->ext.layerObject.x = roomX;
    roomY += bg->yPos.whole;
    entity->ext.layerObject.y = roomY;
    entity->ext.layerObject.drawX = roomX - bg->xPos.whole;
    entity->ext.layerObject.drawY = roomY - bg->yPos.whole;
    entity->scrollWithBg = 0;
    if ((s32)entity->ext.layerObject.shuttleStart > entity->ext.layerObject.shuttleEnd)
        entity->ext.layerObject.shuttleDir = 1;
    else
        entity->ext.layerObject.shuttleDir = 0;
}

/**
 * @brief 59580 | Object 0x32, state 0: keep the world position (ext.layerObject.x /
 * ext.layerObject.y) on screen; when it becomes standingPlatform crumble in four timed steps
 * (frames animFrame + 1, sounds 0x152, 0x151, 0x150) and break into debris.
 */
void CrumblingPlatform_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];

    entity->ext.layerObject.drawX = entity->ext.layerObject.x - bg->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - bg->yPos.whole;
    switch (entity->subState)
    {
        case 0:
            if ((*ewramRef)->playerState.standingPlatform != entity)
                break;
            entity->subState++;
            entity->timer = 0;
            PlaySong(0x152);
            break;
        case 1:
            if (entity->timer++ < 10)
                break;
            entity->subState++;
            entity->timer = 0;
            entity->animFrame++;
            break;
        case 2:
            if (entity->timer++ < 0x14)
                break;
            entity->subState++;
            entity->timer = 0;
            entity->animFrame++;
            PlaySong(0x151);
            break;
        case 3:
            if (entity->timer++ < 10)
                break;
            entity->subState++;
            entity->timer = 0;
            entity->animFrame++;
            PlaySong(0x150);
            break;
        case 4:
            if (entity->timer++ < 6)
                break;
            entity->deleteRequested = 1;
            CrumblingPlatform_SpawnDebris(entity, 5, 1);
            CrumblingPlatform_SpawnDebris(entity, 6, 2);
            CrumblingPlatform_SpawnDebris(entity, 4, 3);
            if (entity->spawnParam1 == 0)
            {
                CrumblingPlatform_SpawnDebris(entity, 4, 0);
                CrumblingPlatform_SpawnDebris(entity, 5, 4);
            }
            break;
    }
    entity->posX.whole = entity->ext.layerObject.x - bg->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - bg->yPos.whole;
}

/**
 * @brief 596E0 | Spawn debris piece index (frame frame) of object 0x32:
 * x offset (index * 6 - 12) px, flying sideways by its index with random
 * upward speed and flips, gravity 0x1800.
 */
void CrumblingPlatform_SpawnDebris(struct EwramData_EntityData *entity, s32 frame, s32 index)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, CrumblingPlatformDebris_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + (s16)(index * 6 - 12) * 0x10000;
    child->posY.whole = entity->posY.whole + 0xC0000;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sCrumblingPlatformGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sCrumblingPlatformSpriteSheet, gfx, child->basePalette);
    child->animFrame = frame;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->velX.whole = (index << 13) - 0x4000;
    child->velY.whole = (RandomNumberGenerator() & 0x3FFF) + 0x4000;
    child->accelY.whole = 0x1800;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief 597DC | Object 0x32 update: Player_QueueEntity(1) while the player is
 * within 0x20 px.
 */
void CrumblingPlatformUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;
    s32 dx;
    struct HitResult result;

    if (entity->state == 0)
        CrumblingPlatform_UpdateState(entity);
    player = gEwramData->playerState.player;
    dx = player->posX.whole - entity->posX.whole;
    if (dx < 0)
        dx = -dx;
    if (dx <= 0x1FFFFF)
    {
        s32 dy = player->posY.whole - entity->posY.whole;
        if (dy < 0)
            dy = -dy;
        if (dy <= 0x1FFFFF)
            Player_QueueEntity(entity, 1);
    }
    Hit_CheckWithPlayer(&result, entity);
}

/**
 * @brief 59840 | Debris piece: fall with gravity 0x40000 max, deleted when
 * outside the screen.
 */
void CrumblingPlatformDebris_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x40000);
    if (EntityIsOutsideScreen(entity, 3))
        EntityDelete(entity);
}

/**
 * @brief 59864 | Load the graphics of object 0x32; 0 when no slot is free.
 */
u8 CrumblingPlatform_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sCrumblingPlatformGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sClockTowerPlatformPalette, 1);
    Sprite_Attach(entity, sCrumblingPlatformSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 598B0 | Object 0x33 create (spark source): load the graphics
 * (delete the object when that fails), animation 1 for variant 1, else 0;
 * variant 0 is hidden.
 */
void SparkEmitterCreate(struct EwramData_EntityData *entity)
{
    u16 script;

    if (!SparkEmitter_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    if (entity->spawnParam0 != 1)
        script = 0;
    else
        script = 1;
    Anim_StartIndexed(entity, script, 0, 1);
    Anim_Update(entity);
    entity->objMode = 1;
    entity->spriteLayer = 9;
    entity->flipX = 0;
    entity->flipY = 0;
    entity->state = 0;
    if (entity->spawnParam0 == 0)
        entity->hidden = 1;
}

/**
 * @brief 59928 | Spawn a spark (SparkEmitterFlyingSpark_Update) flying up-left (variant 2:
 * random left/right) with random frame and flips, gravity 0x2800, 0x32
 * frames.
 */
void SparkEmitter_SpawnFlyingSpark(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, SparkEmitterFlyingSpark_Update);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole;
    child->posY.whole = entity->posY.whole;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sSparkEmitterGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sSparkEmitterSpriteSheet, gfx, child->basePalette);
    child->animFrame = RandomNumberGenerator() % 4 + 0xB;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->objMode = 1;
    if (entity->spawnParam0 == 2)
        child->velX.whole = RandomNumberGenerator() % 0x40000 - 0x20000;
    else
        child->velX.whole = -0x10000 - RandomNumberGenerator() % 0x10000;
    if (entity->spawnParam0 == 0)
        child->velY.whole = -0x10000 - RandomNumberGenerator() % 0x10000;
    else
        child->velY.whole = -0x20000 - RandomNumberGenerator() % 0x10000;
    child->accelY.whole = 0x2800;
    child->ext.raw.unk_4FC.whole = bg->xPos.whole;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
    child->timer = 0x32;
}

/**
 * @brief 59A8C | Spawn a spark (SparkEmitterFallingSpark_Update) within +-16 px with random
 * frame 6..10, flips and a small upward speed.
 */
void SparkEmitter_SpawnFallingSpark(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, SparkEmitterFallingSpark_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole + RandomNumberGenerator() % 0x200000 - 0x100000;
    child->posY.whole = entity->posY.whole;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sSparkEmitterGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sSparkEmitterSpriteSheet, gfx, child->basePalette);
    child->animFrame = RandomNumberGenerator() % 5 + 6;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->flipX = RandomNumberGenerator() % 2;
    child->flipY = RandomNumberGenerator() % 2;
    child->objMode = 1;
    child->velX.whole = RandomNumberGenerator() % 0xC000 - 0x6000;
    child->velY.whole = -0x4000 - RandomNumberGenerator() % 0x2000;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
    child->timer = 0x32;
}

/**
 * @brief 59B9C | Object 0x33 update.
 */
void SparkEmitterUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        SparkEmitter_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 59BBC | Spark (SparkEmitter_SpawnFallingSpark): fall, blink, end after timer.
 */
void SparkEmitterFallingSpark_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x60000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

/**
 * @brief 59C04 | Load the graphics of object 0x33; 0 when no slot is free.
 */
u8 SparkEmitter_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sSparkEmitterGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sSparkEmitterPalette, 0);
    Sprite_Attach(entity, sSparkEmitterSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 59C50 | Object 0x33, state 0: every 0x10..0x1F frames spawn a spark
 * (and for variants != 0 a second kind).
 */
void SparkEmitter_UpdateState(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if (entity->timer-- > 1)
        return;
    entity->timer = RandomNumberGenerator() % 16 + 0x10;
    SparkEmitter_SpawnFlyingSpark(entity);
    if (entity->spawnParam0 != 0)
        SparkEmitter_SpawnFallingSpark(entity);
}

/**
 * @brief 59C90 | Spark (SparkEmitter_SpawnFlyingSpark): slow down sideways, fall, blink,
 * end after timer.
 */
void SparkEmitterFlyingSpark_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x40);
    EntityAccelerateAndMove(entity, 0x60000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

/**
 * @brief 59CE0 | Object 0x34 create (swinging platform): load the graphics
 * (delete the object when that fails), angle unk_4FC = 0x4000, swing
 * acceleration unk_500 = spawnParam1, start direction by variant, world
 * position in unk_4F4/unk_4F8; spawn its three parts.
 */
void PendulumPlatformCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 roomX;
    s32 roomY;

    if (!PendulumPlatform_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->animFrame = 3;
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->ext.pendulumPlatform.angle = 0x4000;
    entity->ext.pendulumPlatform.accel = entity->spawnParam1;
    if (entity->spawnParam0 == 0)
    {
        entity->subState = 0;
        entity->ext.pendulumPlatform.speed = 0x100;
    }
    else
    {
        entity->subState = 1;
        entity->ext.pendulumPlatform.speed = -0x100;
    }
    roomX = entity->posX.whole + bg->xPos.whole;
    entity->ext.layerObject.x = roomX;
    roomY = entity->posY.whole + bg->yPos.whole;
    entity->ext.layerObject.y = roomY;
    entity->ext.layerObject.drawX = roomX - bg->xPos.whole;
    entity->ext.layerObject.drawY = roomY - bg->yPos.whole;
    entity->scrollWithBg = 0;
    PendulumPlatform_SpawnPart(entity, 0);
    PendulumPlatform_SpawnPart(entity, 1);
    PendulumPlatform_SpawnPart(entity, 2);
}

/**
 * @brief 59D98 | Object 0x34, state 0: swing the angle ext.pendulumPlatform.angle by the
 * speed (+-0x100, accelerated by ext.pendulumPlatform.accel)->ext.pendulumPlatform.speed between 0x3FFF and
 * 0x4000 turning points; keep the world position on screen.
 */
void PendulumPlatform_UpdateSwing(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 speed;

    entity->ext.layerObject.drawX = entity->ext.layerObject.x - bg->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - bg->yPos.whole;
    switch (entity->subState)
    {
        case 0:
            {
                u16 speed2 = entity->ext.pendulumPlatform.speed;
                if (entity->ext.pendulumPlatform.speed > -0x100)
                {
                    entity->ext.pendulumPlatform.speed = speed2 - entity->ext.pendulumPlatform.accel;
                    if (entity->ext.pendulumPlatform.speed < -0x100)
                        entity->ext.pendulumPlatform.speed = -0x100;
                }
            }
            entity->ext.pendulumPlatform.angle += entity->ext.pendulumPlatform.speed;
            if (entity->ext.pendulumPlatform.angle > 0x3FFF)
                break;
            entity->subState = 1;
            break;
        case 1:
            speed = entity->ext.pendulumPlatform.speed;
            if (entity->ext.pendulumPlatform.speed <= 0xFF)
            {
                entity->ext.pendulumPlatform.speed = speed + entity->ext.pendulumPlatform.accel;
                if (entity->ext.pendulumPlatform.speed > 0x100)
                    entity->ext.pendulumPlatform.speed = 0x100;
            }
            entity->ext.pendulumPlatform.angle += entity->ext.pendulumPlatform.speed;
            if (entity->ext.pendulumPlatform.angle <= 0x4000)
                break;
            entity->subState = 0;
            break;
    }
    entity->posX.whole = entity->ext.layerObject.x - bg->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - bg->yPos.whole;
}

/**
 * @brief 59E4C | Spawn part part (0, 1: chain links, 2: the platform
 * with a hit box) of object 0x34 at its position; the links get an affine
 * slot rotated by the parent's angle. ext.pendulumPlatformPart.pivot = parent, ext.pendulumPlatformPart.index = index.
 */
void PendulumPlatform_SpawnPart(struct EwramData_EntityData *entity, s32 part)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, PendulumPlatformPart_Update);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u32 box = 0x1C20F4F0;
    s32 gfx;
    u16 affineIndex;
    s32 roomX;
    s32 roomY;

    if (child == NULL)
        return;
    child->posX.whole = entity->posX.whole;
    child->posY.whole = entity->posY.whole;
    child->spriteLayer = entity->spriteLayer + 1;
    gfx = ObjGfx_Find(sPendulumPlatformGfx);
    child->basePalette = entity->basePalette;
    Sprite_Attach(child, sPendulumPlatformSpriteSheet, gfx, child->basePalette);
    if (part == 0)
        child->animFrame = 2;
    else if (part == 1)
        child->animFrame = part;
    else
        child->animFrame = 0;
    if (part == 2)
    {
        Hit_Enable(child, 8, NULL, 0);
        HitBox_SetBody(child, &box);
        HitBox_SetAttack(child, &box);
    }
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    if (part != 2)
    {
        affineIndex = ObjAffine_Alloc(1);
        if ((s16)affineIndex < 0)
        {
            EntityDelete(child);
            return;
        }
        child->affineIndex = affineIndex;
        child->affine = 1;
        ObjAffine_SetBiased(entity->ext.pendulumPlatform.angle, 0x10000, 0x10000, child->flipX, child->flipY, child->affineIndex);
    }
    roomX = child->posX.whole + bg->xPos.whole;
    child->ext.layerObject.x = roomX;
    roomY = child->posY.whole + bg->yPos.whole;
    child->ext.layerObject.y = roomY;
    child->ext.layerObject.drawX = roomX - bg->xPos.whole;
    child->ext.layerObject.drawY = roomY - bg->yPos.whole;
    child->ext.pendulumPlatformPart.pivot = entity;
    child->ext.pendulumPlatformPart.index = part;
    child->scrollWithBg = 0;
    child->drawFunc = PendulumPlatformPart_Draw;
}

/**
 * @brief 59FB4 | Object 0x34 part: hang at distance (index * 30 + 16) px
 * from the parent along its swing angle (moving by the difference so that
 * riders follow); links rotate with the parent; the platform (index 2)
 * calls Player_QueueEntity(1) near the player and carries the player
 * (Player_SetXFraction / Player_SetYFraction) while being standingPlatform.
 */
void PendulumPlatformPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];
    struct EwramData_EntityData *pivot = entity->ext.pendulumPlatformPart.pivot;
    struct EwramData_EntityData *player;
    s32 targetX;
    s32 targetY;
    s32 dist;

    entity->ext.layerObject.x += entity->velX.whole;
    entity->ext.layerObject.y += entity->velY.whole;
    if (entity->subState == 0)
    {
        targetX = Sin(pivot->ext.pendulumPlatform.angle + 0x4000);
        targetX = FixedMultiply(targetX, ((s8)entity->ext.pendulumPlatformPart.index * 15 << 17) + 0x100000);
        targetY = Sin(pivot->ext.pendulumPlatform.angle);
        targetY = FixedMultiply(targetY, ((s8)entity->ext.pendulumPlatformPart.index * 15 << 17) + 0x100000);
        targetX += pivot->ext.layerObject.x;
        targetY += pivot->ext.layerObject.y;
        entity->velX.whole = targetX - entity->ext.layerObject.x;
        entity->velY.whole = targetY - entity->ext.layerObject.y;
    }
    if ((s8)entity->ext.pendulumPlatformPart.index != 2)
        ObjAffine_SetBiased(pivot->ext.pendulumPlatform.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    entity->posX.whole = entity->ext.layerObject.x - bg->xPos.whole;
    entity->posY.whole = entity->ext.layerObject.y - bg->yPos.whole;
    entity->ext.layerObject.drawX = entity->ext.layerObject.x - bg->xPos.whole;
    entity->ext.layerObject.drawY = entity->ext.layerObject.y - bg->yPos.whole;
    if ((s8)entity->ext.pendulumPlatformPart.index == 2)
    {
        player = (*ewramRef)->playerState.player;
        dist = player->posX.whole - entity->posX.whole;
        if (dist < 0)
            dist = -dist;
        if (dist <= 0x1FFFFF)
        {
            dist = player->posY.whole - entity->posY.whole;
            if (dist < 0)
                dist = -dist;
            if (dist <= 0x1FFFFF)
                Player_QueueEntity(entity, 1);
        }
    }
    if (gEwramData->playerState.standingPlatform == entity)
    {
        // Low halves of posX / posY, as in the original.
        Player_SetXFraction((s16)entity->posX.part16.fraction);
        Player_SetYFraction((s16)entity->posY.part16.fraction);
    }
}

/**
 * @brief 5A108 | Object 0x34 update.
 */
void PendulumPlatformUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
        PendulumPlatform_UpdateSwing(entity);
}

/**
 * @brief 5A120 | Load the graphics of object 0x34; 0 when no slot is free.
 */
u8 PendulumPlatform_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sPendulumPlatformGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sClockTowerPlatformPalette, 2);
    Sprite_Attach(entity, sPendulumPlatformSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5A16C | Object 0x34 part draw: draw (Sprite_Draw) at the
 * position saved in ext.layerObject.drawX / _Y, then restore posX / posY.
 */
void PendulumPlatformPart_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = entity->ext.layerObject.drawX;
    entity->posY.whole = entity->ext.layerObject.drawY;
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
