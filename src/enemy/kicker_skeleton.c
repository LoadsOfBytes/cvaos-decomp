/*
 * Kicker Skeleton.
 */
#include "core.h"
#include "sound.h"
#include "player_core.h"
#include "graphics.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"

/* Kicker Skeleton leap, kick, landing and recovery. */
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern u8 sKickerSkeletonWalkProfile[];
extern s32 __divsi3(s32, s32);

/**
 * @brief Attack state (1): prepare, leap and dive-kick with afterimages,
 * then land and brake before returning to idle. Kick flag 1 controls
 * the enhanced contact damage. Preserve the halfword animation-event test.
 */
void KickerSkeleton_UpdateKick(struct EwramData_EntityData *entity)
{
    s32 flags;
    s32 x;
    s32 y;
    s32 timer;
    struct EwramData_EntityData *particle;
    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        entity->subState = 1;
        /* fall through */
    case 1:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 2;
        break;
    case 2:
        if (!(entity->animFlags & 4)) break;
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->ext.kickerSkeleton.speed = 0x8000;
        entity->velY.whole = -0x30000;
        entity->accelY.whole = 0x1000;
        entity->subState = 3;
        break;
    case 3:
        flags = Entity_Walk(entity, entity->ext.kickerSkeleton.speed, sKickerSkeletonWalkProfile, 0);
        if (flags & 4) entity->subState = 7;
        else if (entity->velY.whole >= -0x18000)
        {
            Entity_FaceTarget(entity);
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 4;
        }
        break;
    case 4:
        Entity_Walk(entity, entity->ext.kickerSkeleton.speed, sKickerSkeletonWalkProfile, 0);
        /* The ROM tests the frame and its adjacent byte as one halfword. */
        if (*(u16 *)&entity->animStep != 5) break;
        Anim_StartIndexed(entity, 5, 0, 0);
        Afterimage_Create(entity, 0x30, 0xC, 0x11);
        Afterimage_Create(entity, 0x30, 6, 0x10);
        entity->ext.kickerSkeleton.speed = 0x28000;
        entity->velY.whole = 0x30000;
        entity->ext.kickerSkeleton.flags |= 1;
        PlaySong(0x87);
        entity->subState = 5;
        break;
    case 5:
        flags = Entity_Walk(entity, entity->ext.kickerSkeleton.speed, sKickerSkeletonWalkProfile, 0);
        if (flags & 8)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->ext.kickerSkeleton.flags &= ~1;
            PlaySong(0x127);
            entity->subState = 6;
        }
        break;
    case 6:
        flags = Entity_Walk(entity, entity->ext.kickerSkeleton.speed, sKickerSkeletonWalkProfile, 0);
        {
            s32 speed = entity->ext.kickerSkeleton.speed;
            s32 value = 31;
            value *= speed;
            entity->ext.kickerSkeleton.speed = __divsi3(value, 32);
        }
        if ((s32)entity->ext.kickerSkeleton.speed <= 0x7FFF) goto finished;
        if (!(flags & 8)) break;
        if (flags & 3) entity->ext.kickerSkeleton.speed = 0;
        if ((s32)entity->ext.kickerSkeleton.speed <= 0xBFFF) break;
        timer = entity->timer + 1;
        entity->timer = timer;
        if (timer & 3) break;
        x = entity->posX.whole + (RandomSigned(0x10) << 16);
        y = entity->posY.whole + (RandomSigned(4) << 16) + 0xC0000;
        particle = Effect_SpawnShared(x, y, (RandomNumberGenerator() & 1) + 4, 6);
        if (particle == NULL) break;
        particle->drawSkip = 1;
        particle->velX.whole = 0x8000;
        if (entity->flipX) particle->velX.whole = -particle->velX.whole;
        particle->accelY.whole = -(RandomNumberGenerator() & 0x3FF);
        break;
    case 7:
        flags = Entity_Walk(entity, entity->ext.kickerSkeleton.speed, sKickerSkeletonWalkProfile, 0);
        if (flags & 8)
        {
finished:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}

/* Kicker Skeleton breaks into seven sprite fragments (frames 26..32). */
extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *);
extern u8 sKickerSkeletonGfx[], sKickerSkeletonSpriteSheet[];

/**
 * @brief Spawn seven sprite fragments (frames 26 through 32) with random
 * velocities and stored attacker-relative horizontal direction. Return
 * nonzero immediately so the dispatcher removes the enemy.
 */
s32 KickerSkeleton_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *fragment;
    s32 frame;
    s32 graphics;
    PlaySong(0x6B);
    for (frame = 0x1A; frame <= 0x20; frame++)
    {
        fragment = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateSpriteDebris);
        if (fragment == NULL) continue;
        fragment->posX.whole = entity->posX.whole;
        fragment->posY.whole = entity->posY.whole;
        fragment->velX.whole = entity->ext.kickerSkeleton.hitSide * (((RandomNumberGenerator() & 7) << 14) + 0x8000);
        fragment->velY.whole = (((RandomNumberGenerator() & 0xF) - 8) << 12) - 0x28000;
        fragment->accelY.whole = 0x2000;
        fragment->spriteLayer = entity->spriteLayer;
        graphics = ObjGfx_Find(sKickerSkeletonGfx);
        Sprite_Attach(fragment, sKickerSkeletonSpriteSheet, graphics, entity->basePalette);
        fragment->flipX = entity->flipX;
        fragment->drawFunc = (u32 *)Sprite_DrawEntity;
        fragment->scrollWithBg = 1;
        fragment->timer = (RandomNumberGenerator() & 0xF) + 0x28;
        fragment->animFrame = frame;
    }
    return 1;
}

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void KickerSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *);
void KickerSkeleton_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sKickerSkeletonGfx[], sKickerSkeletonPalette[], sKickerSkeletonSpriteSheet[];

void EnemyKickerSkeletonCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sKickerSkeletonGfx, sKickerSkeletonPalette, 0, sKickerSkeletonSpriteSheet))
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
        Entity_EnableBoxes(entity, KickerSkeleton_DamagePlayerOnContact, KickerSkeleton_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer -= 0x10;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);

void EnemyKickerSkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0:
        if (entity->subState == 0)
        {
            if (!(RandomNumberGenerator() & 0x3F))
                Anim_StartIndexed(entity, RandomNumberGenerator() & 1, 0, 1);
            if (Target_IsNearEntity(entity, 0xC8, 0x50) || Enemy_IsDamaged(entity))
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
        }
        break;
    case 1: KickerSkeleton_UpdateKick(entity); break;
    case 2:
        if (KickerSkeleton_UpdateDeath(entity))
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

/* KickerSkeleton_OnHit: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, store the attacker-
 * relative fragment direction, restore the palette and enter death state 2.
 */
void KickerSkeleton_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, (s32)attacker, hit))
    {
        Enemy_DropRewards(entity);
        {
            s32 speed = 1;

            entity->ext.kickerSkeleton.hitSide = speed;
            if (entity->posX.whole < attacker->posX.whole)
                entity->ext.kickerSkeleton.hitSide = -speed;
        }
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
