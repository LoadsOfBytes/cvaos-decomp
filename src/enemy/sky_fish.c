/*
 * Sky Fish: body and trails. The last part of the range holds the entry points of Gremlin.
 */
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "player_core.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "gargoyle.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "entity_hit.h"

void SkyFish_UpdateFly(struct EwramData_EntityData *);
void SkyFish_UpdateDeath(struct EwramData_EntityData *);
u8 SkyFish_LoadGraphics(struct EwramData_EntityData *);
void SkyFish_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void SkyFish_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
void SkyFish_SpawnTrail(struct EwramData_EntityData *, s32, s32, s32, s32);
void SkyFish_SpawnRisingTrail(struct EwramData_EntityData *, s32, s32, s32, s32);

/* States of SkyFish (entity->state), named after the update function the dispatcher calls. */
enum {
    SKY_FISH_STATE_FLY = 0,
    SKY_FISH_STATE_DEATH = 1,
    SKY_FISH_STATE_ANIM_ONLY = 2,
};

void EnemySkyFishUpdate(struct EwramData_EntityData *entity)
{
    /* Original reserves 0x30 bytes for this scratch area. */
    struct HitResult results[3]; /* only results[0] is written; the original reserved 48 bytes */
    u8 frame;
    if (entity->state != SKY_FISH_STATE_DEATH)
    {
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 1 && entity->ext.skyFish.darting == 0)
            Hit_CheckWithPlayer(results, entity);
        return;
    }
    switch (entity->state)
    {
    case SKY_FISH_STATE_FLY: SkyFish_UpdateFly(entity); break;
    case SKY_FISH_STATE_DEATH: SkyFish_UpdateDeath(entity); break;
    case SKY_FISH_STATE_ANIM_ONLY: Anim_Update(entity); return;
    }
    frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 1 && entity->ext.skyFish.darting == 0)
        Hit_CheckWithPlayer(results, entity);
}

void EnemySkyFishCreate(struct EwramData_EntityData *entity)
{
    s32 one;
    if (!SkyFish_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_Start(entity, &sSkyFishFlyAnimScript, 0, 0);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = SKY_FISH_STATE_ANIM_ONLY;
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, SkyFish_DamagePlayerOnContact, SkyFish_OnHit);
    else
        Hit_Enable(entity, 8, SkyFish_DamagePlayerOnContact, SkyFish_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = SKY_FISH_STATE_FLY;
    one = 1;
    entity->ext.skyFish.darting = one;
    {
        u32 random = RandomNumberGenerator();
        s32 mask = 0x3F;
        mask &= random;
        entity->ext.skyFish.delay = mask + 0x40;
    }
    entity->hidden = 1;
    entity->ext.skyFish.fromRight = RandomNumberGenerator() & one;
}

/* Sky Fish: normal movement and death sequence. */

/* Fast traversal; SkillChronomageUse sets and decrements the global 0x4BE
 * countdown. While active it switches this fish to a slower animation/movement
 * state, rather than simply stopping its updates. */
void SkyFish_UpdateFly(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((s8)entity->ext.skyFish.fromRight != 0)
            entity->posX.whole = 0x1000000;
        else
            entity->posX.whole = -0x100000;
        entity->posY.whole = (RandomNumberGenerator() % 0x200000) + 0x200000;
        entity->flipX = Entity_IsTargetRight(entity);
        if (gEwramData->gameState.timeStopTimer != 0)
            entity->deleteRequested = 1;
        else if (entity->ext.skyFish.delay-- <= 1)
        {
            entity->subState++;
            entity->hidden = 0;
            if (entity->flipX)
                entity->velX.whole = 0x1A0000;
            else
                entity->velX.whole = -0x1A0000;
        }
        break;
    case 1:
        {
            s32 x = entity->posX.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            s32 y = entity->posY.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            EntityMove(entity);
            SkyFish_SpawnTrail(entity, x, y, entity->spriteLayer + 1, 1);
            if (gEwramData->gameState.timeStopTimer != 0)
            {
                entity->subState++;
                entity->ext.skyFish.darting = 0;
                Anim_StartIndexed(entity, 0, 0, 1);
                if (entity->flipX)
                    entity->velX.whole = 0x10000;
                else
                    entity->velX.whole = -0x10000;
            }
            else if (EntityIsOutsideScreen(entity, 6))
                entity->deleteRequested = 1;
        }
        break;
    case 2:
        EntityMove(entity);
        if (gEwramData->gameState.timeStopTimer == 0)
        {
            Anim_Start(entity, &sSkyFishFlyAnimScript, 0, 0);
            entity->subState = 1;
            entity->ext.skyFish.darting = 1;
            if (entity->flipX)
                entity->velX.whole = 0x1A0000;
            else
                entity->velX.whole = -0x1A0000;
        }
        break;
    }
}

/* Fall, blink, emit a small effect every eight frames, then end. */
void SkyFish_UpdateDeath(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        entity->subState++;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x2000;
    }
    EntityAccelerateAndMove(entity, 0x40000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((entity->timer & 7) == 0)
    {
        s32 x = entity->posX.whole + (RandomNumberGenerator() % 0x80000) - 0x40000;
        s32 y = entity->posY.whole + (RandomNumberGenerator() % 0x100000) - 0x80000;
        SkyFish_SpawnRisingTrail(entity, x, y, entity->spriteLayer + 1, 0);
    }
    if ((s8)--entity->timer <= 0)
        entity->deleteRequested = 1;
}

void SkyFish_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
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
        Anim_Start(entity, &sSkyFishHitAnimScript, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x40;
        entity->state = SKY_FISH_STATE_DEATH;
        entity->subState = 0;
    }
}

/* Create a moving animated effect. Position arguments use 16.16 fixed point. */
void SkyFish_SpawnTrail(struct EwramData_EntityData *parent, s32 x, s32 y, s32 priority, s32 param)
{
    struct EwramData_EntityData *child = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, Effect_UpdateParticle);
    if (child != NULL)
    {
        s32 tiles;
        child->posX.whole = x;
        child->posY.whole = y;
        child->velX.whole = 0;
        child->velY.whole = 0x4000;
        child->accelX.whole = 0;
        child->accelY.whole = 0;
        child->ext.particle.damping = 0x20;
        child->spriteLayer = priority;
        child->timer = 0;
        child->ext.particle.flags = param;
        if (parent->flipX)
            child->velX.whole = 0x20000;
        else
            child->velX.whole = -0x20000;
        tiles = ObjGfx_Find(sSkyFishGfx);
        child->basePalette = parent->basePalette;
        Sprite_Attach(child, sSkyFishSpriteSheet, tiles, child->basePalette);
        Anim_StartIndexed(child, 1, 0, 0);
        Anim_Update(child);
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
    }
}

/* Companion effect with upward acceleration. */
void SkyFish_SpawnRisingTrail(struct EwramData_EntityData *parent, s32 x, s32 y, s32 priority, s32 param)
{
    struct EwramData_EntityData *child = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, Effect_UpdateParticle);
    if (child != NULL)
    {
        s32 tiles;
        child->posX.whole = x;
        child->posY.whole = y;
        child->velX.whole = 0;
        child->velY.whole = 0;
        child->accelX.whole = 0;
        child->accelY.whole = -0x1000;
        child->ext.particle.damping = 0x18;
        child->spriteLayer = priority;
        child->timer = 0;
        child->ext.particle.flags = param;
        tiles = ObjGfx_Find(sSkyFishGfx);
        child->basePalette = parent->basePalette;
        Sprite_Attach(child, sSkyFishSpriteSheet, tiles, child->basePalette);
        Anim_StartIndexed(child, 1, 0, 0);
        Anim_Update(child);
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SkyFish_LoadGraphics = LegionWalkingCorpse_LoadGraphics
 *   SkyFish_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief A8194 | Same as LegionWalkingCorpse_LoadGraphics.
 */
u8 SkyFish_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sSkyFishGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sSkyFishPalette, 0);
    Sprite_Attach(entity, sSkyFishSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief A81E0 | Same as Enemy_DamagePlayerOnContact.
 */
void SkyFish_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyGremlinUpdate(struct EwramData_EntityData *entity)
{
    EnemyGargoyleUpdate(entity);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyGremlinCreate(struct EwramData_EntityData *entity)
{
    EnemyGargoyleCreate(entity);
}
