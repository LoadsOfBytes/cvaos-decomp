/*
 * Zombie Soldier: body, grenades and pieces.
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "gameplay.h"
#include "sound.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"

extern void ZombieSoldier_UpdateWalk(struct EwramData_EntityData *entity);
extern void ZombieSoldier_UpdateAttack(struct EwramData_EntityData *entity);
extern void ZombieSoldier_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame);

/**
 * @brief 926F8 | Zombie Soldier update: skipped by Enemy_PreUpdate; states 0
 * / 1 (ZombieSoldier_UpdateWalk / ZombieSoldier_UpdateAttack), 2 dying: on frame 0x28 two random
 * body parts ZombieSoldier_SpawnPiece (frames 0x13 / 0x14, velX away from the facing),
 * 3 animation only. The 0x30-byte stack buffer is unused in the original.
 */
void EnemyZombieSoldierUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[0x30];
    s32 x, y, vx;

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
    case 0:
        ZombieSoldier_UpdateWalk(entity);
        break;
    case 1:
        ZombieSoldier_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer == 0x28)
        {
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -0x10000 - (RandomNumberGenerator() & 0xFFFFF);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieSoldier_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x13);
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -(RandomNumberGenerator() % 0x180000);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieSoldier_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x14);
        }
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyZombieSoldierCreate = EnemySkeletonKnightCreate */

extern u8 ZombieSoldier_LoadGraphics(struct EwramData_EntityData *entity);
extern void ZombieSoldier_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void ZombieSoldier_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 92868 | Same as EnemySkeletonKnightCreate.
 */
void EnemyZombieSoldierCreate(struct EwramData_EntityData *entity)
{
    if (!ZombieSoldier_LoadGraphics(entity))
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
        Hit_EnableContact(entity, 8, ZombieSoldier_DamagePlayerOnContact, (s32)ZombieSoldier_OnHit);
    else
        Hit_Enable(entity, 8, ZombieSoldier_DamagePlayerOnContact, (s32)ZombieSoldier_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

extern s32 Target_IsInRect(s16 x, s16 y, s32 w, s32 h);
extern void Entity_MoveWithCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit, s32 wallOffset);

/**
 * @brief 928FC | Zombie Soldier state 0 (walk): while on the ground
 * (animStep 1 / 5, animTimer clear) it faces the player and, with the player
 * in an 80x70 area in front, attacks with 1/2 chance (state 1, animation
 * 1), or with 1/4 chance when within 220x70 (state 1 with sub-state 10,
 * animation 2); otherwise walks at 0x4000 / frame. Stopped in the air
 * (animStep 0 / 4); moves via Entity_MoveWithCollision.
 */
void ZombieSoldier_UpdateWalk(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if ((entity->animStep == 1 || entity->animStep == 5) && entity->animTimer == 0)
    {
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect(entity->posX.part16.integer - 0x28, entity->posY.part16.integer - 0x40, 0x50, 0x46)
            && (RandomNumberGenerator() & 0x7F) <= 0x3F)
        {
            entity->state = 1;
            entity->subState = 0;
            Anim_StartIndexed(entity, 1, 0, 0);
            return;
        }
        if ((RandomNumberGenerator() & 0x7F) <= 0x1F
            && Target_IsInRect(entity->posX.part16.integer - 0x6E, entity->posY.part16.integer - 0x40, 0xDC, 0x46))
        {
            entity->state = 1;
            entity->subState = 10;
            Anim_StartIndexed(entity, 2, 0, 0);
            return;
        }
        if (entity->flipX)
            entity->velX.whole = 0x4000;
        else
            entity->velX.whole = -0x4000;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
    }
    if ((entity->animStep == 0 || entity->animStep == 4) && entity->animTimer == 0)
        entity->velX.whole = 0;
    Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
}

/* ZombieSoldier_UpdateAttack: enemy attack states (throws ZombieSoldier_ThrowGrenade); matched. */
/* Throw a projectile toward the player's horizontal position, then return
 * to the idle state when the animation ends. */
extern void ZombieSoldier_ThrowGrenade(struct EwramData_EntityData *owner, s32 x, s32 y, s32 velX, s32 velY);

/**
 * @brief Attack state machine (state 1): melee swing (sound 0x85) or, from
 * sub-state 10, throw a grenade (ZombieSoldier_ThrowGrenade); then back to
 * the walk state.
 */
void ZombieSoldier_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 ky = -0x250000;

    switch (entity->subState)
    {
        case 0:
            entity->subState = 1;
            break;
        case 1:
            if (*(u16 *)&entity->animStep == 2)
                PlaySong(0x85);
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
                entity->velX.whole = 0;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            break;
        case 10:
            entity->flipX = Entity_IsTargetRight(entity);
            if (*(u16 *)&entity->animStep == 10)
            {
                s32 offset;
                s32 vx;

                offset = entity->flipX ? 0x100000 : -0x100000;
                vx = (Target_GetX(entity) - (entity->posX.whole + offset)) / 44;
                if (entity->flipX)
                {
                    if (vx > 0x13FFF)
                        vx = 0x14000;
                }
                else if (vx <= -0x14000)
                {
                    vx = -0x14000;
                }
                ZombieSoldier_ThrowGrenade(entity, offset, ky, vx, -0x20000);
                entity->subState++;
            }
            break;
        case 11:
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
                entity->velX.whole = 0;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            break;
    }
}

/* ZombieSoldier_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */

extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief 92B38 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x70, animation 3, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void ZombieSoldier_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
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
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Spawn the projectile ZombieSoldierGrenade_Update at the owner + (x, y) with the given
 * velocity, copying the owner's facing, palette and layer. */

extern void ZombieSoldierGrenade_Update(struct EwramData_EntityData *);
extern void ZombieSoldierGrenade_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void ZombieSoldierGrenade_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern u8 sZombieSoldierGfx[];
extern u8 sZombieSoldierSpriteSheet[];

/**
 * @brief Throw a grenade from owner + (x, y) with (velX, velY) under gravity;
 * contact ZombieSoldierGrenade_DamagePlayerOnContact, hit
 * ZombieSoldierGrenade_OnHit.
 */
void ZombieSoldier_ThrowGrenade(struct EwramData_EntityData *owner, s32 x, s32 y, s32 velX, s32 velY)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombieSoldierGrenade_Update);
    struct SpriteBox box = {-2, -2, 4, 4};
    if (shot != NULL)
    {
        s32 graphics;
        shot->posX.whole = owner->posX.whole + x;
        shot->posY.whole = owner->posY.whole + y;
        shot->velX.whole = velX;
        shot->velY.whole = velY;
        shot->accelY.whole = 0x1800;
        shot->flipX = owner->flipX;
        shot->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sZombieSoldierGfx);
        shot->basePalette = owner->basePalette;
        Sprite_Attach(shot, sZombieSoldierSpriteSheet, graphics, shot->basePalette);
        shot->animFrame = 0x12;
        Hit_Enable(shot, 8, ZombieSoldierGrenade_DamagePlayerOnContact, (s32)ZombieSoldierGrenade_OnHit);
        HitBox_SetAttack(shot, &box);
        HitBox_SetBody(shot, &box);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->enemyId = owner->enemyId;
        shot->attack = owner->attack;
        shot->scrollWithBg = 1;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
        shot->timer = 0;
    }
}

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

struct HitRect8 {
    s32 x:8;
    s32 y:8;
    u32 w:8;
    u32 h:8;
};

/**
 * @brief 92CCC | Falling spark bomb: state 0 bounces with Entity_MoveWithPointCollision
 * (velY / 3, ceiling -0x40000), reversing velY on bit 2 (song 0x11B on the
 * first bounce) and exploding (state 2, song 0x76) on the second bounce or on
 * landing; deleted off screen. States 1/2 emit random sparks (Effect_SpawnFadingParticle,
 * every odd frame Effect_SpawnParticleAnim0, a burst Effect_SpawnConfiguredParticle on frame 3); state 2
 * also grows a centred hit box (corner -2 - frame / 2) for 15 frames and
 * dies after 0x17 frames.
 */
void ZombieSoldierGrenade_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    struct HitRect8 rect = { -12, -12, 24, 24 };
    s32 res;
    s32 r, a, b;
    s32 v;
    
    if (entity->state == 0)
    {
        res = Entity_MoveWithPointCollision(entity, entity->velY.whole / 3, -0x40000);
        if (res & 4)
        {
            entity->velY.whole = -entity->velY.whole;
            if (++entity->timer > 1)
                goto land;
            PlaySong(0x11B);
        }
        if (res & 8)
        {
        land:
            entity->state = 2;
            PlaySong(0x76);
            return;
        }
        if (EntityIsOutsideScreen(entity, 2))
            goto delete;
        goto hit;
    }
    else if (entity->state == 1)
    {
        entity->hidden = 1;
        entity->timer++;
        r = RandomNumberGenerator() & 0xFFFF;
        a = Sin(r + 0x4000);
        b = Sin(r);
        a = FixedMultiply(a, 0x10000);
        b = FixedMultiply(b, 0x10000);
        Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer, 1);
        if (entity->timer == 3)
            Effect_SpawnConfiguredParticle(entity->posX.whole, entity->posY.whole, 0, 0, entity->spriteLayer, 1, 7, 5);
        if (entity->timer & 1)
        {
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer);
        }
        if (entity->timer <= 6)
            return;
        goto delete;
    }
    else if (entity->state == 2)
    {
        entity->hidden = 1;
        entity->timer++;
        if (entity->timer <= 8)
        {
            r = RandomNumberGenerator() & 0xFFFF;
            a = Sin(r + 0x4000);
            b = Sin(r);
            a = FixedMultiply(a, 0x10000);
            b = FixedMultiply(b, 0x10000);
            Effect_SpawnFadingParticle(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer, 1);
            if (entity->timer == 3)
                Effect_SpawnConfiguredParticle(entity->posX.whole, entity->posY.whole, 0, 0, entity->spriteLayer, 1, 7, 5);
            if (entity->timer & 1)
            {
                r = RandomNumberGenerator() & 0xFFFF;
                a = Sin(r + 0x4000);
                b = Sin(r);
                a = FixedMultiply(a, 0x10000);
                b = FixedMultiply(b, 0x10000);
                Effect_SpawnParticleAnim0(entity->posX.whole, entity->posY.whole, a, b, entity->spriteLayer);
            }
        }
        if (entity->timer <= 14)
        {
            rect.x = 0xFE - entity->timer / 2;
            rect.y = 0xFE - entity->timer / 2;
            rect.w = (rect.x < 0 ? -rect.x : rect.x) * 2;
            rect.h = (rect.y < 0 ? -rect.y : rect.y) * 2;
            HitBox_SetAttack(entity, &rect);
            entity->hit.flags |= 4;
        }
        if (entity->timer > 0x17)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        if (entity->timer <= 0x12)
        {
        hit:
            Hit_CheckWithPlayer(buffer, entity);
        }
        else
            entity->hit.flags |= 2;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ZombieSoldier_SpawnPiece = ZombieOfficer_SpawnPiece */

extern void ZombieSoldierPiece_Update(struct EwramData_EntityData *entity);

/**
 * @brief 92FCC | Same as ZombieOfficer_SpawnPiece.
 */
void ZombieSoldier_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombieSoldierPiece_Update);

    if (child != NULL)
    {
        s32 gfx;

        child->posX.whole = entity->posX.whole + x;
        child->posY.whole = entity->posY.whole + y;
        child->ext.zombiePiece.floorY = entity->posY.whole;
        child->flipX = entity->flipX;
        child->spriteLayer = entity->spriteLayer - 1;
        gfx = ObjGfx_Find(sZombieSoldierGfx);
        Sprite_Attach(child, sZombieSoldierSpriteSheet, gfx, entity->basePalette);
        child->animFrame = frame;
        child->accelY.whole = 0x1800;
        child->velX.whole = vx;
        child->velY.whole = vy;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Zombie Soldier helpers 0x08093078..0x08093177. */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of Zombie Soldier. */
extern u8 sZombieSoldierPalette[];

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
 * @brief 93078 | Fall (max 0x80000); delete below y unk_500.
 */
void ZombieSoldierPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->posY.whole > entity->ext.zombiePiece.floorY)
        EntityDelete(entity);
}

/**
 * @brief 93098 | Contact: Player_ApplyContactDamage(attack, 2, 0, 1); from state 0 go to
 * state 1 with sound 0x76.
 */
void ZombieSoldierGrenade_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    if (entity->state == 0)
    {
        entity->state = 1;
        entity->timer = 0;
        PlaySong(0x76);
    }
}

/**
 * @brief 930E0 | Hit: hit effect, sound 0x76, state 1.
 */
void ZombieSoldierGrenade_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    PlaySong(0x76);
    entity->state = 1;
    entity->subState = 0;
    entity->timer = 0;
}

/**
 * @brief 93100 | Load the graphics of Zombie Soldier; 0 when no slot is free.
 */
u8 ZombieSoldier_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sZombieSoldierGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sZombieSoldierPalette, 0);
    Sprite_Attach(entity, sZombieSoldierSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 9314C | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void ZombieSoldier_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
