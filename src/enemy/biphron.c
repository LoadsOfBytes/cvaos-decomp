/*
 * Biphron: create, walk, death, attack, shots and flames, debris and graphics.
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "entity_hit.h"

u8 Biphron_LoadGraphics(struct EwramData_EntityData *entity);
void Biphron_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Biphron_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void Biphron_SpawnDebris(struct EwramData_EntityData *entity, s32 x, s32 y, s32 layer, s32 mode);
void Biphron_ThrowShot(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);
void BiphronFlame_Update(struct EwramData_EntityData *entity);
void BiphronShot_Update(struct EwramData_EntityData *entity);
void BiphronShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyBiphronCreate = EnemySkeletonKnightCreate */

/**
 * @brief 99C5C | Same as EnemySkeletonKnightCreate.
 */
void EnemyBiphronCreate(struct EwramData_EntityData *entity)
{
    if (!Biphron_LoadGraphics(entity))
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
        Hit_EnableContact(entity, 8, Biphron_DamagePlayerOnContact, Biphron_OnHit);
    else
        Hit_Enable(entity, 8, Biphron_DamagePlayerOnContact, Biphron_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

/* Walking motion with a delayed chance to begin the nearby-player action. */
void Biphron_UpdateWalk(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) entity->velX.whole = 0x4000;
        else entity->velX.whole = -0x4000;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        entity->subState++;
        /* fall through */
    case 1:
        if (Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8) & 4)
        {
            if (entity->timer++ > 59)
            {
                entity->timer = 0;
                entity->subState = 0;
                if (Target_IsInRect((s16)(entity->posX.part16.integer - 90), (s16)(entity->posY.part16.integer - 100), 180, 200))
                {
                    if ((RandomNumberGenerator() & 127) <= 47)
                    {
                        entity->state = 1;
                        entity->subState = 0;
                    }
                }
            }
        }
        break;
    }
}

#define FRAME(e) (*(u16 *)&(e)->animStep)

/**
 * @brief 99DB8 | Enemy attack animation: on frame 1 spawn two dust puffs
 * (Effect_SpawnParticleAnim0), on frame 6 start an 0x50-frame phase that every 16th frame
 * (while timer > 0x2F) throws a debris piece (Biphron_SpawnDebris, sound 0x6E)
 * and blinks before deleting itself.
 */
void Biphron_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x, y;

    switch (entity->subState)
    {
    case 0:
        if (FRAME(entity) == 1)
        {
            Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, 0, -0x6000, entity->spriteLayer - 1);
            x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            y = entity->posY.whole;
            Effect_SpawnParticleAnim0(x, y, 0, -0x4000, entity->spriteLayer - 1);
            entity->subState++;
        }
        break;
    case 1:
        if (FRAME(entity) == 6)
        {
            entity->subState = 2;
            entity->timer = 0x50;
        }
        break;
    case 2:
        if (entity->timer > 0x2F && (entity->timer & 0xF) == 0)
        {
            x = entity->flipX ? 0x80000 : -0x80000;
            x = x + entity->posX.whole + (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = entity->posY.whole - 0x80000;
            Biphron_SpawnDebris(entity, x, y, entity->spriteLayer - 1, 4);
            PlaySong(0x6E);
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->deleteRequested = 1;
        break;
    }
}

/* Biphron helpers 0x08099ED8..0x0809A197. */
/* animStep and animTimer read as one halfword (as in enemy/harpy.c). */
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

/**
 * @brief 99ED8 | Attack: animation 1; at frame 4 spawn the projectile
 * (Biphron_ThrowShot) 0x1A px in front and play sound 0x78; when the
 * animation ends go back to state 0 with animation 0.
 */
void Biphron_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState++;
            break;
        case 1:
            if (FRAME_PAIR(entity) == 4)
            {
                Biphron_ThrowShot(entity, entity->flipX ? 0x1A0000 : -0x1A0000, 0);
                PlaySong(0x78);
            }
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
            }
            break;
    }
}

/**
 * @brief 99F50 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, animation 3, palette into
 * 0x53E bits 3-6, state 2).
 */
void Biphron_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)attacker, hit, Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief 99FD0 | Spawn the projectile (update BiphronShot_Update, hit callback
 * BiphronShot_DamagePlayerOnContact) at the entity position plus (offsetX, offsetY), thrown
 * forwards (velX +-0x20000, velY 0x40000, accelY 0x2800).
 */
void Biphron_ThrowShot(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, BiphronShot_Update);
    struct SpriteBox box = { -5, -14, 0x14, 0x0E };
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = entity->posX.whole + offsetX;
        child->posY.whole = entity->posY.whole + offsetY;
        child->spriteLayer = entity->spriteLayer;
        gfx = ObjGfx_Find(sBiphronGfx);
        child->basePalette = entity->basePalette;
        Sprite_Attach(child, sBiphronSpriteSheet, gfx, child->basePalette);
        child->hidden = 1;
        Hit_Enable(child, 8, BiphronShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(child, &box);
        child->hit.flags |= 4;
        child->hit.flags &= ~2;
        Entity_SnapToGround(child);
        child->enemyId = entity->enemyId;
        child->attack = entity->attack;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
        child->flipX = entity->flipX;
        if (child->flipX)
            child->velX.whole = 0x20000;
        else
            child->velX.whole = -0x20000;
        child->velY.whole = 0x40000;
        child->accelY.whole = 0x2800;
    }
}

/**
 * @brief 9A0DC | Spawn the projectile's effect (update BiphronFlame_Update) at the
 * entity position plus (offsetX, offsetY), facing like the entity.
 */
void BiphronShot_SpawnFlame(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, BiphronFlame_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = entity->posX.whole + offsetX;
        child->posY.whole = entity->posY.whole + offsetY;
        child->spriteLayer = entity->spriteLayer;
        gfx = ObjGfx_Find(sBiphronGfx);
        child->basePalette = entity->basePalette;
        Sprite_Attach(child, sBiphronSpriteSheet, gfx, child->basePalette);
        Anim_Start(child, (struct AnimScript *)sBiphronShotAnimScript, 0, 0);
        Anim_Update(child);
        child->hit.flags |= 4;
        child->hit.flags |= 2;
        child->enemyId = entity->enemyId;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
        child->flipX = entity->flipX;
    }
}

/* x and y are function-level variables (globally allocated, as in the original);
 * the statement expression keeps -0x100000 next to the random part. */
#define EMIT()                                                                                    \
    {                                                                                             \
        x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;                   \
        y = entity->posY.whole + ({ s32 d = (RandomNumberGenerator() & 0x7FFFF) - 0x100000; d; }); \
        Effect_SpawnTimedFrameParticle(x, y, 0, -0x2000, entity->spriteLayer - 1, 0, 0, 7, 0x18);                       \
    }

/**
 * @brief 9A198 | Randomly flip, emit a particle on animation frames 1 and
 * 0x101, then delete when the animation ends or advance it.
 */
void BiphronFlame_Update(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    s32 one;
    u16 *frame;
    u32 mask;
    s32 x, y;

    {
        u32 r = RandomNumberGenerator();
        one = 1;
        r &= one;
        entity->flipX = r;
    }
    frame = (u16 *)&entity->animStep;
    mask = 0xFFFF;
    if (*frame == 1)
        EMIT();
    mask &= *frame;
    if (mask == 0x101)
        EMIT();
    /* animDone; cse tests it with the shared constant 1 (r9) */
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/* BiphronShot_Update (Biphron projectile update). */

/**
 * @brief 9A29C | Projectile update: move (Entity_MoveWithCollisionDefault); delete when it
 * stops, lands (bit 2 clear) or leaves the screen; otherwise hitbox and
 * every 4 frames an effect (BiphronShot_SpawnFlame) with sound 0x77.
 */
void BiphronShot_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    s32 x = (s16)entity->posX.part16.integer;

    if (!(Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000) & 4)
        || x == (s16)entity->posX.part16.integer
        || EntityIsOutsideScreen(entity, 1))
    {
        EntityDelete(entity);
    }
    else
    {
        Hit_CheckWithPlayer(&result, entity);
        if (!(entity->timer++ & 3))
        {
            BiphronShot_SpawnFlame(entity, 0, 0);
            PlaySong(0x77);
        }
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BiphronShot_DamagePlayerOnContact = GreatArmorShockwave_DamagePlayerOnContact */

/**
 * @brief 9A310 | Same as GreatArmorShockwave_DamagePlayerOnContact.
 */
void BiphronShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

void Biphron_SpawnDebris(struct EwramData_EntityData *parent, s32 x, s32 y, s32 layer, s32 mode)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    if (effect != NULL)
    {
        s32 slot;
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->velX.whole = 0;
        effect->velY.whole = -0x10000;
        effect->accelX.whole = 0;
        effect->accelY.whole = 0;
        effect->spriteLayer = layer;
        effect->timer = 0;
        effect->ext.particle.flags = mode;
        slot = ObjGfx_Find(sBiphronGfx);
        effect->basePalette = parent->basePalette;
        Sprite_Attach(effect, sBiphronSpriteSheet, slot, effect->basePalette);
        Anim_StartIndexed(effect, 4, 0, 0);
        Anim_Update(effect);
        effect->scrollWithBg = 1;
        effect->drawFunc = Sprite_DrawEntity;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Biphron_LoadGraphics = Object25_LoadGraphics
 *   Biphron_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 9A3D8 | Same as Object25_LoadGraphics.
 */
u8 Biphron_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sBiphronGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sBiphronPalette, 0);
    Sprite_Attach(entity, sBiphronSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 9A424 | Same as Enemy_DamagePlayerOnContact.
 */
void Biphron_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 9A450 | Biphron update: state 0..2 run Biphron_UpdateWalk / Biphron_UpdateAttack /
 * Biphron_UpdateDeath then movement (Entity_AnimateWithBoxes, Enemy_CheckHitPlayer); state 3 only
 * Anim_Update. Nothing while Enemy_PreUpdate reports the entity inactive. */
void EnemyBiphronUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[0x30]; /* stack frame reserved by the original */

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            Biphron_UpdateWalk(entity);
            break;
        case 1:
            Biphron_UpdateAttack(entity);
            break;
        case 2:
            Biphron_UpdateDeath(entity);
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
