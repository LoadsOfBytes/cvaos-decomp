/*
 * Evil Butcher: body, projectiles and debris (the last part also holds Kicker Skeleton contact damage).
 */
#include "evil_butcher.h"
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "sound.h"
#include "structs/ewram.h"
#include "entity_hit.h"
#include "player_core.h"
#include "structs/entity_render_flags.h"
#include "death.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_palette.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "bg_and_collision.h"


void EvilButcherProjectile_OnHit(struct EwramData_EntityData *);
void EvilButcher_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/* Thrown projectile: flies sideways until it is destroyed (burst) or hits a
 * wall, then bounces off spinning and falls out of the screen. */
void EvilButcherProjectile_Update(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    u16 slot;

    if (entity->state != 0 && (s16)entity->hp != 0 && Entity_IsTimeStopped(entity))
    {
        Hit_CheckWithPlayer(&res, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)sEvilButcherGfx, (void *)sEvilButcherPalette, entity->spawnParam1, (void *)sEvilButcherSpriteSheet))
            goto delete;
        entity->animFrame = 8;
        entity->spriteLayer = 0x13;
        entity->scrollWithBg = 1;
    {
        s32 vx = -0x28000;
        s16 spin;
        entity->velX.whole = vx;
        entity->ext.evilButcherProjectile.probeX = -6;
        spin = 0x1234;
        entity->ext.evilButcherProjectile.spin = spin;
        if (entity->flipX)
        {
            entity->velX.whole = -vx;
            entity->ext.evilButcherProjectile.probeX = 6;
            entity->ext.evilButcherProjectile.spin = -spin;
        }
    }
        PlaySong(0x89);
        entity->state = 1;
        entity->hp = 1;
        Hit_Enable(entity, 10, Enemy_OnContactMask1, EvilButcherProjectile_OnHit);
        HitBox_SetAttackRaw(entity, 0x0810FCF8);
        HitBox_SetBodyRaw(entity, 0x0810FCF8);
        break;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        Hit_CheckWithPlayer(&res, entity);
        if (EntityIsOutsideScreen(entity, 3))
            entity->state = 3;
        if ((s16)entity->hp == 0)
        {
            entity->state = 3;
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x40000, 0x40000, entity->spriteLayer);
            break;
        }
    {
        struct BgLayerState *camera;
        if (!Collision_GetGroundOffsetWide((s16)entity->posX.part16.integer + (s16)(camera = &gEwramData->bgInfo[1])->xPos.part16.integer + entity->ext.evilButcherProjectile.probeX,
                          (s16)entity->posY.part16.integer + (s16)camera->yPos.part16.integer))
            break;
    }
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        PlaySong(0x13A);
        entity->hp = 0;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->velX.whole = -(entity->velX.whole / 2 + (RandomNumberGenerator() & 0x7FFF));
        entity->velY.whole = RandomSigned(0x8000) - 0x18000;
        entity->accelY.whole = 0x2000;
        entity->state = 2;
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        entity->ext.evilButcherProjectile.angle += entity->ext.evilButcherProjectile.spin;
        ObjAffine_Set(entity->ext.evilButcherProjectile.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
        if (!EntityIsOutsideScreen(entity, 3))
            break;
    case 3:
        if (entity->affineIndex || entity->affine)
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        break;
    }
}

/* Bouncing fragment (EvilButcherDebris_Update): falls until it hits the floor, then
 * flies off spinning with a random speed and direction; deleted off screen. */

void EvilButcherDebris_Update(struct EwramData_EntityData *entity)
{
    u16 slot;

    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sEvilButcherGfx, sEvilButcherPalette, entity->spawnParam1, sEvilButcherSpriteSheet))
                goto delete;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->doubleSize = 1;
            entity->animFrame = 8;
            entity->spriteLayer = 0x12;
            entity->scrollWithBg = 1;
            entity->ext.evilButcherDebris.angle = 0xC000;
            entity->state = 1;
            entity->velY.whole = 0;
            entity->accelY.whole = 0x2000;
            break;
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            if (EntityIsOutsideScreen(entity, 3))
            {
                entity->state = 3;
                break;
            }
            {
                s32 x = (s16)entity->posX.part16.integer;
                struct BgLayerState *camera = &gEwramData->bgInfo[1];
                if (!Collision_GetGroundOffsetWide(x + (s16)camera->xPos.part16.integer,
                                  (s16)entity->posY.part16.integer + (s16)camera->yPos.part16.integer + 4))
                    break;
            }
            PlaySong(0x13A);
            entity->ext.evilButcherDebris.spin = RandomSigned(0x800) + 0x1000;
            entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) + 0x8000;
            if (RandomNumberGenerator() & 1)
            {
                entity->ext.evilButcherDebris.spin = -entity->ext.evilButcherDebris.spin;
                entity->velX.whole = -entity->velX.whole;
            }
            entity->velY.whole = RandomSigned(0x8000) - 0x20000;
            entity->state = 2;
        case 2:
            EntityAccelerateAndMove(entity, 0x80000);
            entity->ext.evilButcherDebris.angle += entity->ext.evilButcherDebris.spin;
            if (!EntityIsOutsideScreen(entity, 3))
                break;
        case 3:
            if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
                ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    ObjAffine_Set(entity->ext.evilButcherDebris.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/* Evil Butcher walking, throwing and death effects. */

/**
 * @brief Walk state (0): vary speed by animation, turn after cycles or
 * terrain obstruction, and enter throw state 1 when the target is near.
 */
void EvilButcher_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 odd;
    if (entity->subState != 0) return;
    if (*(u16 *)&entity->animStep == 0)
    {
        if (Target_IsNearEntity(entity, 0xA0, 0x78))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
            return;
        }
        if (entity->timer++ > 2)
        {
            entity->timer = 0;
            entity->flipX ^= 1;
        }
    }
    odd = entity->animStep;
    odd &= 1;
    speed = 0xA000 - (entity->animTimer << 11);
    if (odd) speed = 0x3000;
    if ((Entity_Walk(entity, speed, sEvilButcherWalkProfile, 0xC) & 0x1B) != 8
        && entity->timer <= 2)
        entity->timer = 3;
}

/**
 * @brief Throw state (1): face the target, spawn a projectile at the
 * combined animation-step/timer event 0x202 and return to walking
 * when the animation finishes.
 */
void EvilButcher_UpdateThrow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *projectile;
    u16 offset;
    s32 offsetX;
    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 0x202)
        {
            projectile = Entity_CreateObject(EvilButcherProjectile_Update);
            if (projectile != NULL)
            {
                offset = -0x10;
                projectile->posX.part16.integer = entity->posX.part16.integer;
                projectile->posY.part16.integer = entity->posY.part16.integer - 0x18;
                projectile->flipX = entity->flipX;
                if (projectile->flipX) offset = 0x10;
                offsetX = (s16)offset;
                offsetX += projectile->posX.part16.integer;
                projectile->posX.part16.integer = offsetX;
                projectile->attack = entity->attack;
                projectile->spawnParam1 = entity->spawnParam1;
            }
        }
        if (entity->animFlags & 4) entity->subState = 3;
        break;
    case 3:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}

/**
 * @brief Death state (2): spawn bouncing debris, display frame 6 and emit
 * random fragments or death particles for the countdown. Return nonzero
 * when removal is allowed.
 */
s32 EvilButcher_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *particle;
    s32 offset;
    s32 x;
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x1C1);
        particle = EntityCreateInRange(0x49, 0xCF, EvilButcherDebris_Update);
        if (particle != NULL)
        {
            offset = 0x120000;
            if (entity->flipX) offset = -offset;
            particle->posX.whole = entity->posX.whole + offset + RandomSigned(0x20000);
            particle->posY.whole = entity->posY.whole + RandomSigned(0x20000) - 0x200000;
            particle->scrollWithBg = 1;
            particle->spawnParam1 = entity->spawnParam1;
        }
        entity->animFrame = 6;
        entity->subState = 1;
        entity->timer = 0x40;
        return 0;
    case 1:
        if (!(entity->timer & 3))
        {
            if (RandomNumberGenerator() & 1)
            {
                x = entity->posX.whole + RandomSigned(0x80000);
                Effect_SpawnShared(x, entity->posY.whole + RandomSigned(0x100000) - 0x100000, 0xD, 8);
            }
            else
            {
                particle = EntityCreateInRange(0x49, 0xCF, EvilButcherDeathParticle_Update);
                if (particle != NULL)
                {
                    particle->posX.whole = entity->posX.whole + RandomSigned(0x80000);
                    particle->posY.whole = entity->posY.whole + RandomSigned(0x100000) - 0x100000;
                    particle->timer = (RandomNumberGenerator() & 7) + 8;
                    particle->scrollWithBg = 1;
                    particle->spawnParam1 = entity->spawnParam1;
                }
            }
        }
        if (entity->timer != 0)
        {
            if (entity->timer == 0x10) entity->drawSkip = 1;
            entity->timer--;
        }
        else return 1;
        break;
    }
    return 0;
}

void EnemyEvilButcherCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sEvilButcherGfx, sEvilButcherPalette, entity->spawnParam1, sEvilButcherSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
        entity->state = 3;
    else
    {
        Entity_EnableBoxes(entity, Enemy_OnContact, EvilButcher_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EvilButcher_OnHit = HeadStackSegment_OnHit */

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 2 with substate counters reset.
 */
void EvilButcher_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Clear projectile HP so its update bursts and removes it.
 */
void EvilButcherProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}

void EnemyEvilButcherUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: EvilButcher_UpdateWalk(entity); break;
    case 1: EvilButcher_UpdateThrow(entity); break;
    case 2:
        if (EvilButcher_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    if (entity->state != 2) Entity_AnimateWithBoxes(entity);
}

/**
 * @brief Apply stored contact damage; while kick flag 1 is set, add one
 * quarter damage, use effect flags 3 and apply directional player knockback.
 */
void KickerSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    s32 speed;
    effect.attributes = 0;
    effect.unused = 0;
    if (entity->ext.kickerSkeleton.flags & 1)
    {
        effect.attack = entity->attack + (entity->attack >> 2);
        effect.reaction = 3;
    }
    else
    {
        effect.attack = entity->attack;
        effect.reaction = 0;
    }
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    if (effect.reaction == 3)
    {
        speed = 0x58000;
        if (entity->velX.whole < 0) speed = -speed;
        Player_SetKnockbackVelocity(speed, -0x18000);
    }
}
