/*
 * Entity helpers: sprite part offsets, debris effects, collision profiles, walking, enemy pre-update, target queries, tile classes, velocity / acceleration, particle spawners, object creation, contact damage, steps toward points, timed effects, the water surface probe and the hit effect spawners.
 * The water probe calls Tile_Classify with s32 arguments (cast marked at the call); HitEffect_Spawn is defined with u16 parameters, so entity_util.h is not included.
 */
#include "structs/ewram.h"
#include "graphics.h"
#include "gameplay.h"
#include "sound.h"
#include "bg_and_collision.h"
#include "structs/main.h"
#include "player_core.h"
#include "structs/entity_template.h"
#include "core.h"
#include "syscalls.h"
#include <string.h>
#include "gba/dma.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "effects.h"
#include "data/data_118B48.h"
#include "math_inline.h"
#include "hitbox.h"
#include "damage_numbers.h"
#include "obj_palette.h"
#include "entity_hit.h"

extern s32 Entity_IsInRect(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern const u8 sEntityVisibilityPattern[];
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Hit_GetContactPoint(s32 a, struct EwramData_EntityData *b, s16 *outX, s16 *outY);

void Effect_UpdateFallingFrameParticle(struct EwramData_EntityData *entity);
void Effect_UpdateStationaryAnimation(struct EwramData_EntityData *entity);
void TimedEntity_Update(struct EwramData_EntityData *entity);

/* from entity_util.h (not included: see the module notes) */

/* Flipped offsets of sprite parts, decompiled from original assembly. */
/* Offset to the far corner of a part, counting parts in reverse order. */
void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset)
{
    u8 *graphics = &entity->spriteSlot;
    struct SpriteSheet *animation = gEwramData->spriteSlots[*graphics].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = animation->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)(frameIndex * sizeof(*frames) + (u32)frames);
    u8 count = frame->partCount;
    struct SpritePart *part = frame->parts;
    if (index >= count)
        index = count - 1;
    part += count - index - 1;
    if (entity->flipX)
        offset[0] = -part->x - part->width;
    else
        offset[0] = part->x + part->width;
    if (entity->flipY)
        offset[1] = -part->y - part->height;
    else
        offset[1] = part->y + part->height;
}

/* Offset to the centre of a part. */
/**
 * @brief Return the flipped center offset of the reverse-indexed sprite
 * part, clamping indices above the part count to its last index.
 * Preserve the existing behavior for empty frames and negative indices.
 */
void Sprite_GetPartCenterOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset)
{
    u8 *graphics = &entity->spriteSlot;
    struct SpriteSheet *animation = gEwramData->spriteSlots[*graphics].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = animation->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)(frameIndex * sizeof(*frames) + (u32)frames);
    u8 count = frame->partCount;
    struct SpritePart *part = frame->parts;
    if (index >= count)
        index = count - 1;
    part += count - index - 1;
    if (entity->flipX)
        offset[0] = -part->x - (part->width >> 1);
    else
        offset[0] = part->x + (part->width >> 1);
    if (entity->flipY)
        offset[1] = -part->y - (part->height >> 1);
    else
        offset[1] = part->y + (part->height >> 1);
}

/* Offset to the part origin, with both entity flips applied. */
/**
 * @brief Return the flipped origin offset of the reverse-indexed sprite
 * part, clamping indices above the part count to its last index.
 * Preserve the existing behavior for empty frames and negative indices.
 */
void Sprite_GetPartOriginOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset)
{
    u8 *graphics = &entity->spriteSlot;
    struct SpriteSheet *animation = gEwramData->spriteSlots[*graphics].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = animation->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)(frameIndex * sizeof(*frames) + (u32)frames);
    u8 count = frame->partCount;
    struct SpritePart *part = frame->parts;
    if (index >= count)
        index = count - 1;
    part += count - index - 1;
    if (entity->flipX)
        offset[0] = -part->x;
    else
        offset[0] = part->x;
    if (entity->flipY)
        offset[1] = -part->y;
    else
        offset[1] = part->y;
}

/* Debris motion, ground impact and particle emission. */

/**
 * @brief Damp X by 79/80, accelerate and move the sprite debris.
 * While state is 0 sample part 0 center plus 4 pixels for ground
 * impact; emit the shared impact particle or remove outside bounds.
 */
void Effect_UpdateSpriteDebris(struct EwramData_EntityData *entity)
{
    s16 offset[2];
    s32 factor = 79;
    s32 speed = entity->velX.whole;
    s32 limit;
    speed *= factor;
    speed /= 80;
    limit = 0x80000;
    entity->velX.whole = speed + entity->accelX.whole;
    if (entity->accelX.whole >= 0)
    {
        if (entity->velX.whole > limit)
            entity->velX.whole = limit;
    }
    else if (entity->velX.whole < -0x80000)
        entity->velX.whole = -0x80000;
    entity->velY.whole += entity->accelY.whole;
    if (entity->accelY.whole >= 0)
    {
        if (entity->velY.whole > limit)
            entity->velY.whole = limit;
    }
    else if (entity->velY.whole < -0x80000)
        entity->velY.whole = -0x80000;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    if (entity->state == 0)
    {
        s16 *point = offset;
        Sprite_GetPartCenterOffset(entity, 0, point);
        entity->ext.spriteDebris.probeX = offset[0];
        entity->ext.spriteDebris.probeY = point[1] + 4;
    }
    {
        s32 x = (s16)entity->posX.part16.integer;
        struct BgLayerState *background = &gEwramData->bgInfo[1];
        if ((s16)Collision_GetGroundOffset(x + (s16)background->xPos.part16.integer + entity->ext.spriteDebris.probeX,
                             entity->posY.part16.integer + (s16)background->yPos.part16.integer + entity->ext.spriteDebris.probeY))
        {
            struct EwramData_EntityData *particle = Effect_SpawnShared(
                entity->posX.whole + (entity->ext.spriteDebris.probeX << 16),
                entity->posY.whole + (entity->ext.spriteDebris.probeY << 16) - 0x40000, 4, 6);
            if (particle != NULL)
            {
                s32 random;
                particle->drawSkip = 1;
                random = RandomSigned(0x1000);
                particle->velY.whole = random - 0x4000;
                particle->accelY.whole = particle->velY.whole / 32;
                particle->velX.whole = RandomSigned(0x2000) - entity->velX.whole;
                particle->accelX.whole = -particle->velX.whole / 64;
            }
            PlaySong(0x162);
            entity->deleteRequested = 1;
        }
        else if (!Entity_IsInRect(entity, -32, -32, 304, 224))
            entity->deleteRequested = 1;
    }
}

/* Sample the old world position, move with damping, then emit the
 * appropriate impact effect for the tile beneath that sampled position. */
/**
 * @brief Sample the old room-space position, then damp X by 39/40
 * and move. Use the sampled ground/tile result to choose an impact
 * effect and request deletion; preserve sampling before movement.
 */
void Effect_UpdateDebrisWithTileImpact(struct EwramData_EntityData *entity)
{
    u16 *selfX = &entity->posX.part16.integer;
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u16 x = *selfX + (u16)background->xPos.part16.integer;
    u16 *selfY = (u16 *)&entity->posY.part16.integer;
    u16 y = *selfY + (u16)background->yPos.part16.integer;
    s32 factor = 39;
    s32 speed = entity->velX.whole;
    s32 limit;
    speed *= factor;
    speed /= 40;
    limit = 0x80000;
    entity->velX.whole = speed + entity->accelX.whole;
    if (entity->accelX.whole >= 0)
    {
        if (entity->velX.whole > limit)
            entity->velX.whole = limit;
    }
    else if (entity->velX.whole < -0x80000)
        entity->velX.whole = -0x80000;
    entity->velY.whole += entity->accelY.whole;
    if (entity->accelY.whole >= 0)
    {
        if (entity->velY.whole > limit)
            entity->velY.whole = limit;
    }
    else if (entity->velY.whole < -0x80000)
        entity->velY.whole = -0x80000;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    if ((s16)Collision_GetGroundOffset((s16)x, (s16)y))
    {
        if (!Collision_IsPointSolid((s16)x, (s16)y))
        {
            Effect_SpawnShared(entity->posX.whole, entity->posY.whole - 0x40000, 0, 7);
            entity->deleteRequested = 1;
        }
        else
        {
            s32 offset = (s16)Collision_GetPushUpFiltered((s16)x, (s16)y, 2);
            if (offset != 0)
                Effect_SpawnShared(entity->posX.whole, entity->posY.whole + (offset << 16), 11, 6);
            entity->deleteRequested = 1;
        }
    }
}

/* Entity motion with tile collision; profile has count, top, bottom,
 * half-width, then count vertical sample offsets. */

/**
 * @brief Move using profile count/top/bottom/half-width and vertical
 * wall samples, optional floor filtering and step size. Return
 * wall/ceiling/floor/slope bits and store the landing tile flags.
 */
s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *entity, s16 *profile, u16 stepSize, s32 flags)
{
    s32 count = profile[0];
    s16 *sample = profile + 4;
    s32 step = 0;
    s32 result = 0;
    u16 x = GetEntityRoomXPositionInteger(entity);
    u16 y = GetEntityRoomYPositionInteger(entity);
    s32 limit = 0x80000;
    s32 limitY;
    s32 zero;
    s32 i;
    u8 *tileFlags;

    entity->velX.whole += entity->accelX.whole;
    if (entity->accelX.whole >= 0)
    {
        if (entity->velX.whole > limit)
            entity->velX.whole = limit;
    }
    else if (entity->velX.whole < -0x80000)
        entity->velX.whole = -0x80000;
    entity->posX.whole += entity->velX.whole;
    if (entity->velX.whole < 0)
    {
        i = 0;
        if (i < count)
        {
            s32 sampleX = (s16)x;
            s32 sampleY = (s16)y;
            do
            {
                s32 delta = Collision_GetPushRight(sampleX - profile[3], sampleY + *sample);
                if (delta != 0)
                {
                    entity->posX.whole += delta << 16;
                    result |= 1;
                    break;
                }
                sample++;
                i++;
            } while (i < count);
        }
    }
    else if (entity->velX.whole > 0)
    {
        i = 0;
        if (i < count)
        {
            s32 sampleX = (s16)x;
            s32 sampleY = (s16)y;
            do
            {
                s32 delta = Collision_GetPushLeft(profile[3] + sampleX, sampleY + *sample);
                if (delta != 0)
                {
                    entity->posX.whole += delta << 16;
                    result |= 2;
                    break;
                }
                sample++;
                i++;
            } while (i < count);
        }
    }
    limitY = 0x80000;
    entity->velY.whole += entity->accelY.whole;
    if (entity->accelY.whole >= 0)
    {
        if (entity->velY.whole > limitY)
            entity->velY.whole = limitY;
    }
    else if (entity->velY.whole < -0x80000)
        entity->velY.whole = -0x80000;
    entity->posY.whole += entity->velY.whole;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    tileFlags = &entity->floorFlags;
    zero = 0;
    *tileFlags = zero;
    if (entity->velY.whole < 0)
    {
        s32 delta = Collision_GetPushDown((s16)x, (s16)y + profile[1]);
        if (delta != 0)
        {
            s32 correction = delta << 16;
            entity->posY.whole += correction;
            result |= 4;
            entity->velY.whole = zero;
        }
    }
    else
    {
        u32 delta;
        if (entity->velY.part16.integer == 0)
            step = stepSize;
        delta = (u16)Collision_GetPushUp((s16)x, (s16)y + profile[2] + (s16)step);
        {
            s32 filterMask = 2;
            if (flags & filterMask)
            {
                s32 tile = gUnk_03002CB0.collisionProbeFlags;
                s32 allowed = 0;
                if ((tile & filterMask) || (tile & 0xC0))
                {
                    s32 kind;
                    if (tile & 8)
                        kind = (tile & 1) ? 2 : 1;
                    else
                        kind = 0;
                    if (kind != 1)
                        allowed = 1;
                }
                if (!allowed)
                    return result;
            }
        }
        if ((s16)delta == 0)
            return result;
        entity->posY.whole += ((s16)delta + (s16)step) << 16;
        result |= 8;
        entity->velY.whole = 0;
        entity->floorFlags = gUnk_03002CB0.collisionProbeFlags;
        if (gUnk_03002CB0.collisionProbeFlags & 0xC0)
        {
            result |= 0x10;
            if ((entity->velX.whole > 0 && (gUnk_03002CB0.collisionProbeFlags & 4))
                || (entity->velX.whole < 0 && !(gUnk_03002CB0.collisionProbeFlags & 4)))
                result |= 0x40;
            else
                result |= 0x20;
        }
        if ((flags & 1) && Collision_GetPushUp((s16)x, (s16)y + profile[2]) != 0
            && (gUnk_03002CB0.collisionProbeFlags & 0xC0))
            result |= 0x10;
    }
    return result;
}

/* Walk with facing/slope adjustment and restore position when collision
 * or the requested floor-support condition rejects movement. */

s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags)
{
    u32 oldX = entity->posX.whole;
    u32 oldY = entity->posY.whole;
    s32 supportMask;
    /* Preserve the original high-register assignment across the collision call. */
    register s32 collisionFlags asm("r10") = 0; /* FAKEMATCH: pinned register */
    s32 hit;
    s32 support;
    if ((flags & 1) && *(u16 *)&entity->animStep == 0)
        Entity_FaceTarget(entity);
    if (!(flags & 2) && !entity->flipX)
        entity->velX.whole = -speed;
    else
        entity->velX.whole = speed;
    if ((flags & 8) && (entity->floorFlags & 0xC0))
    {
        s32 slope = (entity->floorFlags & 0xC0) >> 6;
        if ((entity->velX.whole > 0 && (entity->floorFlags & 4))
            || (entity->velX.whole < 0 && !(entity->floorFlags & 4)))
        {
            switch (slope)
            {
                case 1:
                    entity->velX.whole /= 2;
                    break;
                case 2:
                    entity->velX.whole = entity->velX.whole / 3 * 2;
                    break;
            }
        }
    }
    supportMask = 0x10;
    support = flags & supportMask;
    if (support)
        collisionFlags |= 1;
    if (flags & 0x20)
        collisionFlags |= 2;
    hit = Entity_MoveWithCollisionProfile(entity, profile, 4, collisionFlags);
    if (hit & 3)
    {
        entity->posX.whole = oldX;
        if (hit & 8)
            entity->posY.whole = oldY;
    }
    else if (support && (hit & supportMask))
    {
        entity->posX.whole = oldX;
        entity->posY.whole = oldY;
    }
    if ((flags & 4) && !(hit & 8))
    {
        entity->posX.whole = oldX;
        entity->posY.whole = oldY;
    }
    return hit;
}

/* Common pre-update entity culling and collision callback. */

s32 Enemy_PreUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u32 templateOffset = entity->enemyId * sizeof(struct EntityTemplate);
    struct EntityTemplate *entry = (struct EntityTemplate *)(templateOffset + (u32)sEnemyData);
    if (entry->previewOffsetY == 0)
        Entity_HideAboveY(entity, -16);
    if (entity->enemyFlags & 2)
        return 0;
    if (Entity_IsInRect(entity, -128, -64, 496, 288))
    {
        if (Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (!Enemy_IsStunned(entity))
            return 0;
        Hit_CheckWithPlayer(&result, entity);
    }
    return 1;
}

/* Entity target helpers 0x0806CCB4..0x0806CF2B (no upstream C existed).
 *
 * Most functions read a "target" entity: gEwramData->playerState.linkTargetProxy when
 * gameState.linkPlay is set, else playerState.enemyTarget when non-NULL, else the
 * entity itself (or 0 for velocities). */

/**
 * @brief Repeat RNG modulo the first argument until the result is at
 * least the second. Preserve rejection sampling and the original
 * behavior when arguments cannot produce an accepted result.
 */
u32 RandomModuloAtLeast(u32 modulus, u32 minimum)
{
    u32 randomValue;

    do
    {
        randomValue = RandomNumberGenerator() % modulus;
    } while (randomValue < minimum);
    return randomValue;
}

/**
 * @brief 6CCD0 | Clear byte 0x504 and start sequence 0 via Anim_StartIndexed.
 */
void GiantGhost_ResetAnimation(struct EwramData_EntityData *entity)
{
    entity->ext.giantGhost.unusedFlag = 0;
    Anim_StartIndexed(entity, 0, 0, 0);
}

/**
 * @brief Draw normally, then outside bestiary preview draw another copy
 * 8 pixels right on layer 0x1D. Restore X, Y and sprite layer
 * after the second draw; no specific visual role is asserted.
 */
void Entity_DrawWithOffsetCopy(struct EwramData_EntityData *entity)
{
    s32 oldX;
    s32 oldY;
    u8 oldLayer;

    Sprite_DrawEntity(entity);
    if (!gEwramData->gameState.bestiaryPreview)
    {
        oldX = entity->posX.whole;
        oldY = entity->posY.whole;
        oldLayer = entity->spriteLayer;
        entity->posX.whole = oldX + 0x80000;
        entity->spriteLayer = 0x1D;
        Sprite_DrawEntity(entity);
        entity->posX.whole = oldX;
        entity->posY.whole = oldY;
        entity->spriteLayer = oldLayer;
    }
}

/**
 * @brief 6CD38 | Target x position (16.16).
 */
s32 Target_GetX(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->posX.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->posX.whole;
    return entity->posX.whole;
}

/**
 * @brief 6CD78 | Target y position (16.16).
 */
s32 Target_GetY(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->posY.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->posY.whole;
    return entity->posY.whole;
}

/**
 * @brief 6CDB8 | Target x position, integer part.
 */
s32 Target_GetPixelX(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;

    if (!gEwramData->gameState.linkPlay)
    {
        if (gEwramData->playerState.enemyTarget == NULL)
            target = entity;
        else
            target = gEwramData->playerState.enemyTarget;
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
    }
    return (s16)target->posX.part16.integer;
}

/**
 * @brief 6CDFC | Target y position, integer part.
 */
s32 Target_GetPixelY(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;

    if (!gEwramData->gameState.linkPlay)
    {
        if (gEwramData->playerState.enemyTarget == NULL)
            target = entity;
        else
            target = gEwramData->playerState.enemyTarget;
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
    }
    return target->posY.part16.integer;
}

/**
 * @brief Return alternate-target X velocity when flag linkPlay is set,
 * otherwise primary-target X velocity or 0 when no target exists.
 */
s32 Target_GetVelocityX(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->velX.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->velX.whole;
    return 0;
}

/**
 * @brief Return alternate-target Y velocity when flag linkPlay is set,
 * otherwise primary-target Y velocity or 0 when no target exists.
 */
s32 Target_GetVelocityY(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.linkPlay)
        return gEwramData->playerState.linkTargetProxy->velY.whole;
    if (gEwramData->playerState.enemyTarget != NULL)
        return gEwramData->playerState.enemyTarget->velY.whole;
    return 0;
}

/**
 * @brief Return the primary target horizontal flip, or the inverse
 * of the supplied entity flip if the primary target is NULL.
 * Unlike position/velocity helpers, ignore alternate-target mode.
 */
s32 Target_GetFlipXOrOpposite(struct EwramData_EntityData *entity)
{
    if (gEwramData->playerState.enemyTarget == NULL)
        return entity->flipX ^ 1;
    return gEwramData->playerState.enemyTarget->flipX;
}

/**
 * @brief Return 1 if the primary target exists and its unsigned
 * EntityDistance is at most the supplied limit, else 0.
 * This helper does not select the alternate target.
 */
s32 Target_IsWithinDistance(struct EwramData_EntityData *entity, u32 maxDistance)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;

    if (target == NULL || (u32)EntityDistance(entity, target) > maxDistance)
        return 0;
    return 1;
}

/* Target-relative helpers 0x0806CF2C..0x0806D0B7. Target = playerState.linkTargetProxy when gameState.linkPlay is
 * set, else playerState.enemyTarget. */

// Defined in src/engine/entity_motion.c with u16 parameters; the original caller
// sign-extends the first two coordinates, i.e. it saw an s16 prototype.

/**
 * @brief Return 1 when signed entity X minus target X is negative, meaning the target is to the right; missing target returns entity flipX.
 */
s32 Entity_IsTargetRight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;

    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;

    if (target == NULL)
        return entity->flipX;

    if ((s16)entity->posX.part16.integer - (s16)target->posX.part16.integer >= 0)
        return 0;
    return 1;
}

/**
 * @brief Return 1 when entity Y minus target Y is negative, meaning the target is below; missing target returns a random bit.
 */
s32 Entity_IsTargetBelow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;

    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;

    if (target == NULL)
        return RandomNumberGenerator() & 1;

    if (entity->posY.part16.integer - target->posY.part16.integer >= 0)
        return 0;
    return 1;
}

/**
 * @brief 6CFFC | Whether enemyTarget exists and lies in the given rectangle
 * (see Entity_IsInRect).
 */
s32 Target_IsInRect(u16 left, u16 top, u16 width, u16 height)
{
    if (gEwramData->playerState.enemyTarget == NULL)
        return 0;
    return Entity_IsInRect(gEwramData->playerState.enemyTarget, (s16)left, (s16)top, (u16)width, (u16)height);
}

#define DIFF_INT(a, b) (((a) - (b) < 0) ? -(((b) - (a)) >> 16) : (((a) - (b)) >> 16))

/**
 * @brief Return Euclidean distance to Target_GetX/Y using truncated whole-pixel differences and a signed-halfword result.
 */
s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity)
{
    s32 dx;
    s32 dy;

    dx = DIFF_INT(Target_GetX(entity), entity->posX.whole);
    dx *= dx;
    dy = DIFF_INT(Target_GetY(entity), entity->posY.whole);
    dy *= dy;
    return Sqrt(dx + dy);
}

/* Entity/tile helper routines 0x0806D0B8..0x0806D2F7 (no upstream C existed). */

/* Rectangle with 32-bit fields (x, y, w, h order is a guess from the flips). */
struct FlipRect32 {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
};

/**
 * @brief 6D0B8 | Classify the tile at (x, y) via Collision_GetTile.
 *
 * @return 0 or 1 for an empty tile (depending on the tile below), 2 or 3 for
 * a non-empty one (bit 3 clear / set)
 */
s32 Tile_Classify(s16 x, s16 y)
{
    s32 tile;

    tile = Collision_GetTile(x, y);
    if (tile == 0)
    {
        tile = Collision_GetTile(x, y + 1);
        if (tile & 3)
            return 0;
        return 1;
    }

    if (tile & 8)
        return 3;
    return 2;
}

/**
 * @brief Return 1 when the tile collision flags contain bit 6 or 7, otherwise zero.
 */
s32 Collision_IsSlopeTile(s16 x, s16 y)
{
    s32 tile = Collision_GetTile(x, y);
    if (tile & 0xC0)
        return 1;
    return 0;
}

/**
 * @brief 6D128 | Whether the entity is outside the 240x160 screen extended by
 * margin pair margin of sOffscreenMargins.
 *
 * @return 1 when outside, 0 when inside
 */
s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin)
{
    s16 margins[7][2];
    s32 pos;

    memcpy(margins, sOffscreenMargins, sizeof(margins));

    pos = (s16)entity->posX.part16.integer;
    if (pos >= -margins[margin][0] && pos <= margins[margin][0] + 240)
    {
        pos = entity->posY.part16.integer;
        if (pos >= -margins[margin][1] && pos <= margins[margin][1] + 160)
            return 0;
    }
    return 1;
}

/**
 * @brief Compare room X in eight-pixel tiles with the supplied tile margin; return 1 at the left margin, 2 at the right, otherwise zero.
 */
s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *entity, s32 margin)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct BgMapMetadata *metadata = bg->pBgMetadata;
    s32 tileX;
    s32 widthTiles;

    tileX = ((s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer) / 8;

    if (metadata->widthCells != 1)
        widthTiles = metadata->widthCells << 5;
    else
        widthTiles = 0x1E;

    if (tileX < margin)
        return 1;
    if (tileX > widthTiles - margin)
        return 2;
    return 0;
}

/**
 * @brief Apply horizontal and vertical flip adjustments to the four-word rectangle view; retain its existing inferred field interpretation.
 */
void Entity_ApplyFlipToRect32(struct EwramData_EntityData *entity, struct FlipRect32 *rect)
{
    u8 *flags = (u8 *)&entity->accelY + 4;

    if (*flags & 0x40)
        rect->x = -(rect->width + rect->x);
    if (*flags & 0x80)
        rect->y = -(rect->height + rect->y);
}

/**
 * @brief Apply flip adjustments to a packed SpriteBox; preserve the original vertical write into height rather than y.
 */
void Entity_ApplyFlipToSpriteBox(struct EwramData_EntityData *entity, struct SpriteBox *box)
{
    u8 *flags = (u8 *)&entity->accelY + 4;
    s32 extent;

    if (*flags & 0x40)
    {
        extent = box->x + box->width;
        box->x = -extent;
    }
    if (*flags & 0x80)
    {
        extent = box->y + box->height;
        box->height = -extent;
    }
}

/**
 * @brief 6D244 | Activate the entity box slot with type 8, then call
 * Entity_UpdateBoxes.
 */
void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt)
{
    Hit_Enable(entity, 8, onHit, onHurt);
    Entity_UpdateBoxes(entity);
}

/**
 * @brief Interpret animation steps as SpriteFrame records and return the largest boxCount over stepCount records.
 */
s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity)
{
    struct SpriteFrame *frames;
    s32 maxBoxes;
    s32 i;

    frames = (struct SpriteFrame *)entity->animScript->steps;
    maxBoxes = 0;
    for (i = 0; i < entity->animScript->stepCount; i++)
    {
        if (frames[i].boxCount > maxBoxes)
            maxBoxes = frames[i].boxCount;
    }
    return maxBoxes;
}

/**
 * @brief 6D288 | Entity_MoveWithCollisionAlt with a fixed last argument of 0x80000.
 */
s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset)
{
    return Entity_MoveWithCollisionAlt(entity, landingVelocity, ceilingOffset, 0x80000);
}

/**
 * @brief Return a 16.16 distance from the supplied point to entity position plus BG1 scroll, using whole-pixel differences before square root.
 */
s32 Entity_GetRoomPointDistance(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    s32 dxSquared;
    s32 dySquared;

    dxSquared = DIFF_INT(x, entity->posX.whole + camera->xPos.whole);
    dxSquared *= dxSquared;
    dySquared = DIFF_INT(y, entity->posY.whole + camera->yPos.whole);
    dySquared *= dySquared;
#undef DIFF_INT

    return Sqrt(dxSquared + dySquared) << 16;
}

/* Entity motion helpers 0x0806D2F8..0x0806D5BF (no upstream C existed).
 *
 * Entity fields used (16.16 fixed point): 0x524/0x528 position x/y,
 * 0x52C/0x530 velocity x/y, 0x534/0x538 acceleration x/y. */

/* 0x28 signed bytes at ROM sEntityVisibilityPattern, copied by Entity_GetVisibilityPattern. Declared as
 * bytes: the original calls memcpy instead of an inline copy. */

static inline void AccelerateX(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    entity->velX.whole += entity->accelX.whole;
    if (maxSpeed != 0)
    {
        if (entity->accelX.whole >= 0)
        {
            if (entity->velX.whole > maxSpeed)
                entity->velX.whole = maxSpeed;
        }
        else if (entity->velX.whole < -maxSpeed)
        {
            entity->velX.whole = -maxSpeed;
        }
    }
}

static inline void AccelerateY(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    entity->velY.whole += entity->accelY.whole;
    if (maxSpeed != 0)
    {
        if (entity->accelY.whole >= 0)
        {
            if (entity->velY.whole > maxSpeed)
                entity->velY.whole = maxSpeed;
        }
        else if (entity->velY.whole < -maxSpeed)
        {
            entity->velY.whole = -maxSpeed;
        }
    }
}

static inline void MoveX(struct EwramData_EntityData *entity)
{
    entity->posX.whole += entity->velX.whole;
}

static inline void MoveY(struct EwramData_EntityData *entity)
{
    entity->posY.whole += entity->velY.whole;
}

/**
 * @brief 6D2F8 | Set the velocity from an angle (0x10000 = full turn) and a
 * 16.16 speed, using Sin (sine) and FixedMultiply (fixed multiply).
 */
void EntitySetVelocityFromAngle(struct EwramData_EntityData *entity, s32 angle, s32 speed)
{
    if (speed == 0x10000)
    {
        entity->velX.whole = Sin(angle + 0x4000);
        entity->velY.whole = Sin(angle);
    }
    else
    {
        entity->velX.whole = FixedMultiply(Sin(angle + 0x4000), speed);
        entity->velY.whole = FixedMultiply(Sin(angle), speed);
    }
}

/**
 * @brief 6D344 | Accelerate in both axes, clamping the speed to maxSpeed
 * (0 = no limit).
 */
void EntityAccelerate(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    EntityAccelerateX(entity, maxSpeed);
    EntityAccelerateY(entity, maxSpeed);
}

/**
 * @brief 6D35C | Accelerate horizontally, clamping to maxSpeed (0 = no limit).
 */
void EntityAccelerateX(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateX(entity, maxSpeed);
}

/**
 * @brief 6D384 | Accelerate vertically, clamping to maxSpeed (0 = no limit).
 */
void EntityAccelerateY(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateY(entity, maxSpeed);
}

/**
 * @brief 6D3AC | Move by the velocity in both axes.
 */
void EntityMove(struct EwramData_EntityData *entity)
{
    MoveX(entity);
    MoveY(entity);
}

/**
 * @brief 6D3C0 | Move horizontally by the velocity.
 */
void EntityMoveX(struct EwramData_EntityData *entity)
{
    MoveX(entity);
}

/**
 * @brief 6D3CC | Move vertically by the velocity.
 */
void EntityMoveY(struct EwramData_EntityData *entity)
{
    MoveY(entity);
}

/**
 * @brief 6D3D8 | Accelerate (clamped) and move in both axes.
 */
void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateX(entity, maxSpeed);
    AccelerateY(entity, maxSpeed);
    MoveX(entity);
    MoveY(entity);
}

/**
 * @brief 6D430 | Accelerate (clamped) and move horizontally.
 */
void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateX(entity, maxSpeed);
    MoveX(entity);
}

/**
 * @brief 6D460 | Accelerate (clamped) and move vertically.
 */
void EntityAccelerateAndMoveY(struct EwramData_EntityData *entity, s32 maxSpeed)
{
    AccelerateY(entity, maxSpeed);
    MoveY(entity);
}

/**
 * @brief 6D490 | Damp the horizontal velocity: v = v * (factor - 1) / factor.
 */
void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor)
{
    entity->velX.whole = entity->velX.whole * (factor - 1) / factor;
}

/**
 * @brief 6D4A8 | Damp the vertical velocity: v = v * (factor - 1) / factor.
 */
void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor)
{
    entity->velY.whole = entity->velY.whole * (factor - 1) / factor;
}

/**
 * @brief 6D4C0 | 1 when EWRAM 0x8E bit 6 is clear, timeStopTimer is non-zero and
 * the entity type's info has bit 10 of unk_1C clear; otherwise 0.
 */
s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity)
{
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0
        && !(sEnemyData[entity->enemyId].flags_1C & 0x400))
        return 1;
    return 0;
}

/**
 * @brief Return the signed byte visibility-table entry for index 0..39, or zero outside that range.
 */
s32 Entity_GetVisibilityPattern(s32 step)
{
    s8 pattern[0x28];

    memcpy(pattern, sEntityVisibilityPattern, sizeof(pattern));
    if (step <= 0x27)
    {
        if (step >= 0)
            return pattern[step];
    }
    return 0;
}

/**
 * @brief 6D54C | 1 when the entity's 0x518 value is below unk_C of its type
 * info, else 0.
 */
s32 Enemy_IsDamaged(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp < sEnemyData[entity->enemyId].hp)
        return 1;
    return 0;
}

/**
 * @brief 6D578 | Spawn an RisingEffect_Update entity in slots 0x49..0xCF at
 * (x, y) with a random horizontal flip and spriteLayer = layer.
 */
void Effect_SpawnAt(s32 x, s32 y, s32 layer)
{
    struct EwramData_EntityData *child;

    child = EntityCreateInRange(0x49, 0xCF, RisingEffect_Update);
    if (child != NULL)
    {
        child->posX.whole = x;
        child->posY.whole = y;
        child->flipX = RandomNumberGenerator() % 2;
        child->spriteLayer = layer;
    }
}

/* Particle spawners 0x0806D5C0..0x0806D893. */

/**
 * @brief 6D5C0 | Spawn a RisingEffect_Update particle at a random offset within
 * width x height around (x, y) (16.16), random facing, layer.
 */
void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer)
{
    s32 dx = RandomNumberGenerator() % width - width / 2;
    s32 dy = RandomNumberGenerator() % height - height / 2;
    s32 px = x + dx;
    s32 py = y + dy;
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, RisingEffect_Update);

    if (entity != NULL)
    {
        entity->posX.whole = px;
        entity->posY.whole = py;
        entity->flipX = RandomNumberGenerator() % 2;
        entity->spriteLayer = layer;
    }
}

/**
 * @brief Spawn an Effect_UpdateParticle particle (common effect graphics,
 * animation 0, kind 3) at (x, y) with velocity on the given layer.
 */
void Effect_SpawnParticleAnim0(s32 x, s32 y, s32 vx, s32 vy, s32 layer)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->spriteLayer = layer;
        entity->timer = 0;
        entity->ext.particle.flags = 3;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(entity);
        entity->scrollWithBg = 1;
        entity->drawFunc = Sprite_DrawEntity;
    }
}

/**
 * @brief As Effect_SpawnParticleAnim0 with random facing and animation 0x11.
 */
void Effect_SpawnParticleAnim11(s32 x, s32 y, s32 vx, s32 vy, s32 layer)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->spriteLayer = layer;
        entity->timer = 0;
        entity->ext.particle.flags = 3;
        entity->flipX = RandomNumberGenerator() % 2;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
        Anim_StartIndexed(entity, 0x11, 0, 0);
        Anim_Update(entity);
        entity->scrollWithBg = 1;
        entity->drawFunc = Sprite_DrawEntity;
    }
}

/**
 * @brief Create a countdown particle with frame 0x35, supplied position,
 * velocity, layer, palette, control flags and timer. Preserve the
 * separately stored unk_4FE byte even though this callback ignores it.
 */
void Effect_SpawnTimedFrameParticle(s32 x, s32 y, s32 vx, s32 vy, s32 layer, s32 damping, s32 flags, s32 palette, s32 duration)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Effect_UpdateTimedParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->spriteLayer = layer;
        entity->timer = 0;
        entity->ext.particle.flags = flags;
        entity->ext.particle.damping = damping;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, palette);
        entity->animFrame = 0x35;
        entity->timer = duration;
        entity->scrollWithBg = 1;
        entity->drawFunc = Sprite_DrawEntity;
    }
}

/**
 * @brief 6D7F0 | Spawn a rising (velY -0x4000) Effect_UpdateParticle particle, kind
 * 4, palette slot 6, animation 4 (mode 0) or 5 with animStep = 2.
 */
void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velY.whole = -0x4000;
        entity->spriteLayer = layer;
        entity->timer = 0;
        entity->ext.particle.flags = 4;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 6);
        if (mode == 0)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
        }
        else
        {
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->animStep = 2;
        }
        Anim_Update(entity);
        entity->scrollWithBg = 1;
        entity->drawFunc = Sprite_DrawEntity;
    }
}

/* Effect_SpawnFadingParticle. */

/**
 * @brief Create a standard particle with X/Y damping and visibility
 * fade flags 7, palette 6 and animation 4 or 5 selected by mode.
 * The supplied velocity is retained.
 */
void Effect_SpawnFadingParticle(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.particle.flags = 7;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Create a standard particle with supplied control flags and
 * palette, velocity and layer; mode selects animation 4 or 5.
 */
void Effect_SpawnConfiguredParticle(s32 x, s32 y, s32 velX, s32 velY, s32 spriteLayer, s32 variant, s32 flags, s32 palette)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = spriteLayer;
    child->timer = 0;
    child->ext.particle.flags = flags;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, palette);
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Create the configurable-damping callback particle with supplied
 * damping byte, control flags, palette and velocity. Mode selects
 * animation 4 or 5; preserve the original field assignment order.
 */
void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticleWithDamping);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.particle.flags = flags;
    child->ext.particle.damping = damping;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, palette);
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief 6DA6C | Effect_SpawnRisingParticle at (x, y) moved randomly within a
 * width x height box centred on it.
 */
void Effect_SpawnRisingParticleInBox(s32 x, s32 y, s32 width, s32 height, s32 layer, s32 mode)
{
    x += RandomNumberGenerator() % width - width / 2;
    y += RandomNumberGenerator() % height - height / 2;
    Effect_SpawnRisingParticle(x, y, layer, mode);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Effect_SpawnFadingParticleCopy = Effect_SpawnFadingParticle */

/**
 * @brief Same as Effect_SpawnFadingParticle.
 */
void Effect_SpawnFadingParticleCopy(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateRandomlyAnimatedParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.particle.flags = 7;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Spawn an Effect_UpdateParticle particle (animation 0xC) with
 * velocity, layer, kind and palette.
 */
void Effect_SpawnParticleAnim12(s32 x, s32 y, s32 velX, s32 velY, s32 spriteLayer, s32 flags, s32 palette)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = spriteLayer;
    child->timer = 0;
    child->ext.particle.flags = flags;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, palette);
    Anim_StartIndexed(child, 0xC, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Create a frame-0x35 particle with supplied velocity and
 * gravity 0x2800, palette 6 and the fixed-lifetime falling callback.
 */
void Effect_SpawnFallingFrameParticle(s32 x, s32 y, s32 velX, s32 velY, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateFallingFrameParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->accelX.whole = 0;
    child->accelY.whole = 0x2800;
    child->spriteLayer = layer;
    child->timer = 0;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    child->animFrame = 0x35;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Create an animation-0xA effect at the supplied position and
 * layer with palette 6. Its callback advances animation until
 * completion; this creator performs no initial animation update.
 */
void Effect_SpawnStationaryAnimation(s32 x, s32 y, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateStationaryAnimation);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->spriteLayer = layer;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    Anim_StartIndexed(child, 0xA, 0, 0);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief 6DCC4 | Spawn a falling piece (Effect_UpdateTimedParticle) at (x, y)
 * with velocity (velX, velY), gravity 0x2000, on layer layer,
 * timer 200, palette 6; animFrame 0x40 / 0x41 / 0x42 for piece 0 / 1 / other.
 */
void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateTimedParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->accelY.whole = 0x2000;
    child->spriteLayer = layer;
    child->timer = 0xC8;
    child->ext.particle.flags = 0;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    if (piece == 0)
        child->animFrame = 0x40;
    else if (piece == 1)
        child->animFrame = 0x41;
    else
        child->animFrame = 0x42;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief Spawn an Effect_UpdateParticle particle (animation 0xD, random
 * facing, kind 3, palette 6 + n) with velocity on the given layer.
 */
void Effect_SpawnParticleAnim13(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 paletteOffset)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->flipX = RandomNumberGenerator() % 2;
    child->spriteLayer = layer;
    child->ext.particle.flags = 3;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, paletteOffset + 6);
    Anim_StartIndexed(child, 0xD, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/**
 * @brief 6DE04 | Spawn a TimedEntity_Update entity at (x, y) on
 * layer layer with flags bits 2 and 1 and 0x53D bit 1 set (param_2 ..
 * param_4 are not used).
 */
void Effect_SpawnTimedEntity(s32 x, s32 y, s32 unused2, s32 unused3, s32 unused4, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, TimedEntity_Update);

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->spriteLayer = layer;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->drawFunc = Sprite_DrawEntity;
}

/* Horizontal distance from the selected target, zero without a target. */
/**
 * @brief Return entity X minus selected target X in fixed-point units.
 * Honor forced-player bit enemyFlags & 4 and alternate-target mode;
 * return 0 when the normal primary target is NULL.
 */
s32 Entity_GetTargetOffsetX(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 x, current;
    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else
    {
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == 0)
                x = entity->posX.whole;
            else
                x = target->posX.whole;
            current = entity->posX.whole;
            return current - x;
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
    }
    x = target->posX.whole;
    return entity->posX.whole - x;
}

/* Vertical distance from the selected target, zero without a target. */
/**
 * @brief Return entity Y minus selected target Y in fixed-point units.
 * Honor forced-player bit enemyFlags & 4 and alternate-target mode;
 * return 0 when the normal primary target is NULL.
 */
s32 Entity_GetTargetOffsetY(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;
    s32 x, current;
    if (entity->enemyFlags & 4)
        target = gEwramData->playerState.player;
    else
    {
        if (!gEwramData->gameState.linkPlay)
        {
            target = gEwramData->playerState.enemyTarget;
            if (target == 0)
                x = entity->posY.whole;
            else
                x = target->posY.whole;
            current = entity->posY.whole;
            return current - x;
        }
        else
            target = gEwramData->playerState.linkTargetProxy;
    }
    x = target->posY.whole;
    return entity->posY.whole - x;
}

/* Entity_AnimateWithBoxes. */

/**
 * @brief 6DF20 | Animate; refresh the boxes (Entity_UpdateBoxes) when the frame
 * id animFrame changed.
 */
void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity)
{
    u8 frame = entity->animFrame;

    Anim_Update(entity);
    if (frame != entity->animFrame)
        Entity_UpdateBoxes(entity);
}

/* Entity_IsOffsetOutsideRoom. */
/**
 * @brief 6DF40 | 1 when the point (offsetX, offsetY) relative to the
 * entity lies outside the room of background 1, in 8 px tiles (rooms of one
 * screen are 30 x 26 tiles, larger ones 32 tiles per screen).
 */
s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity, u16 offsetX, u16 offsetY)
{
    struct BgMapMetadata *metadata = gEwramData->bgInfo[1].pBgMetadata;
    s32 tile;
    s32 limit;

    tile = (GetEntityRoomXPositionInteger(entity) + (s16)offsetX) / 8;
    limit = metadata->widthCells == 1 ? 0x1E : metadata->widthCells * 32;
    if (tile >= 0 && tile < limit)
    {
        tile = (GetEntityRoomYPositionInteger(entity) + (s16)offsetY) / 8;
        limit = metadata->heightCells == 1 ? 0x1A : metadata->heightCells * 32;
        if (tile >= 0 && tile < limit)
            return 0;
    }
    return 1;
}

/* Entity helpers 0x0806DFC8..0x0806E13F. */

/* Start of the 0x800-byte buffer at entityScratchBuffer (two halfwords; 0 and 0x1FF
 * after a reset). */
struct EntityHeapHeader {
    u16 tag;
    u16 words;
};

/**
 * @brief Contact callback: Player_ApplyContactDamage with the entity damage
 * attack at its position.
 */
void Entity_OnPlayerContactDamage(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 6DFF8 | Create an entity with update function updateFunc in slots
 * 0x1B..0x48, or 0xD0..0xDF when unk_8E bit 6 is set.
 */
struct EwramData_EntityData *Entity_CreateObject(EntityFunc updateFunc)
{
    struct EwramData_EntityData *child;

    if (!gEwramData->gameState.bestiaryPreview)
        child = EntityCreateInRange(0x1B, 0x48, updateFunc);
    else
        child = EntityCreateInRange(0xD0, 0xDF, updateFunc);
    return child;
}

/**
 * @brief Zero the shared 0x800-byte entity arena and initialize its first block header to tag 0 and word count 0x1FF.
 */
void Heap_Reset(void)
{
    struct EntityHeapHeader *header = (struct EntityHeapHeader *)gEwramData->entityScratchBuffer;

    DMA_FILL_32(3, 0, header, 0x800);
    header->tag = 0;
    header->words = 0x1FF;
}

/**
 * @brief Copy the entire entity arena to its backup and reset only the active first block header to tag 0 and word count 0x1FF.
 */
void Heap_SaveAndReset(void)
{
    struct EntityHeapHeader *header = (struct EntityHeapHeader *)gEwramData->entityScratchBuffer;

    DMA_COPY_32(3, header, gEwramData->entityHeapBackup, 0x800);
    header->tag = 0;
    header->words = 0x1FF;
}

/**
 * @brief Copy the 0x800-byte entity arena from its backup without changing allocator metadata separately.
 */
void Heap_Restore(void)
{
    DMA_COPY_32(3, gEwramData->entityHeapBackup, gEwramData->entityScratchBuffer, 0x800);
}

/**
 * @brief 6E0D0 | Load graphics resource, palette palette (row variant) and
 * animation set sheet for the entity; 0 when a slot is missing.
 */
u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet)
{
    s32 gfx = ObjGfx_LoadResource(resource, 0);
    u8 paletteIndex;

    if (gfx < 0)
        return 0;
    paletteIndex = ObjPalette_FindOrLoadVariant(palette, variant);
    if ((s8)paletteIndex < 0)
        return 0;
    entity->basePalette = paletteIndex;
    Sprite_Attach(entity, sheet, gfx, entity->basePalette);
    entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Set horizontal velocity to the supplied value when flipX is set, otherwise to its negation.
 */
void Entity_SetVelocityXFromFacing(struct EwramData_EntityData *entity, s32 velocity)
{
    if (entity->flipX)
        entity->velX.whole = velocity;
    else
        entity->velX.whole = -velocity;
}

/* Movement probe: restore the position unless the movement result has mask 4 set. */

static inline s32 ProbeMovement(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit)
{
    return Entity_MoveWithCollisionAlt(entity, landingVelocity, ceilingOffset, speedLimit);
}

/**
 * @brief Optionally face the target, set the walk velocity and probe the
 * movement; restore the position unless the probe allows moving.
 * Return the probe result.
 */
s32 Entity_TryWalk(struct EwramData_EntityData *entity, s32 speed, s32 ceilingOffset, s32 velocityY, s32 skipSetup)
{
    s32 savedX = entity->posX.whole;
    s32 savedY = entity->posY.whole;
    s32 result;
    if (skipSetup == 0 && entity->animTimer == 0) Entity_FaceTarget(entity);
    entity->velY.whole = velocityY;
    if (entity->flipX) entity->velX.whole = speed;
    else entity->velX.whole = -speed;
    result = ProbeMovement(entity, 0x10000, ceilingOffset, 0x80000);
    if (!(result & 4))
    {
        entity->posX.whole = savedX;
        entity->posY.whole = savedY;
    }
    return result;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Enemy_OnContact = Minotaur_DamagePlayerOnContact
 *   Enemy_OnContactMask1 = Entity_OnPlayerContactDamage */

/**
 * @brief Apply player contact damage from the entity attack value and position with effect and reaction fields zero.
 */
void Enemy_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Apply player contact damage from the entity attack value and position with effect mask 1 and reaction field zero.
 */
void Enemy_OnContactMask1(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/* Helpers 0x0806E218..0x0806E34F. */

/**
 * @brief 6E218 | Common hit: flash (hitFlashTimer = 8), subtract the damage from
 * the HP and show it; 1 when the HP dropped to 0 or below.
 */
s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
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
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
        return 1;
    return 0;
}

/**
 * @brief 6E268 | Index (6..1) of the highest set bit among bits 5..0 of
 * attributes, 0 when none is set.
 */
u16 Attribute_GetHighestIndex(u16 attributes)
{
    s32 mask = 0x20;
    u16 index = 0;
    s32 i;

    for (i = 6; i > 0; i--)
    {
        if (attributes & mask)
        {
            index = i;
            break;
        }
        mask >>= 1;
    }
    return index;
}

/**
 * @brief 6E29C | Entity_IsInRect of the target (enemyTarget, or linkTargetProxy when
 * linkPlay is set) with a width x height box centred on entity; 0
 * without a target.
 */
s32 Target_IsNearEntity(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    u16 left = entity->posX.part16.integer - (width >> 1);
    u16 top = entity->posY.part16.integer - (height >> 1);

    if (gEwramData->gameState.linkPlay)
        target = gEwramData->playerState.linkTargetProxy;
    if (target == NULL)
        return 0;
    return Entity_IsInRect(target, (s16)left, (s16)top, (u16)width, (u16)height);
}

/**
 * @brief 6E314 | Unless enemyFlags bit 1 is set: Hit_CheckWithPlayer when the box
 * (-16, 0, 0x110, 0xB0) test of Entity_IsInRect succeeds.
 */
void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (!(entity->enemyFlags & 2) && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
        Hit_CheckWithPlayer(&result, entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Entity_PlaceRelativeToCopy = Entity_PlaceRelativeTo */

/**
 * @brief Same as Entity_PlaceRelativeTo.
 */
void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, u16 offsetY)
{
    s16 dx = offsetX;
    s16 dy = offsetY;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = (s32)dy + anchor->posY.part16.integer;
}

/* Helpers 0x0806E3B0..0x0806E43B. */
/**
 * @brief Return alternate-target horizontal flip when linkPlay is
 * set, else primary-target flip or the supplied entity flip.
 * Unlike Target_GetFlipXOrOpposite, this fallback is not inverted.
 */
u32 Target_GetFlipX(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *target;

    if (!gEwramData->gameState.linkPlay)
    {
        target = gEwramData->playerState.enemyTarget;
        if (target == NULL)
            target = entity;
    }
    else
    {
        target = gEwramData->playerState.linkTargetProxy;
    }
    return target->flipX;
}

/**
 * @brief Return integer-pixel Euclidean distance to the supplied local
 * fixed-point position as a fixed-point result. Preserve signed
 * pixel truncation before squaring and the Sqrt result shift.
 */
s32 Entity_DistanceToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s32 dx2 = PixelDiff(x, entity->posX.whole);
    s32 dy;

    dx2 = dx2 * dx2;
    dy = PixelDiff(y, entity->posY.whole);
    return Sqrt(dx2 + dy * dy) << 16;
}

/* Entity_StepTowardPoint. */
#define INT_TOWARD_ZERO(v) ((v) < 0 ? -((-(v)) >> 16) : (v) >> 16)

static inline s32 Square(s32 v)
{
    return v * v;
}

/* Inlined Entity_AngleToPoint (src/soul/red_souls_b.c). */
static inline u16 AngleToPoint(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    return ArcTan2(INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer,
                   INT_TOWARD_ZERO(y) - entity->posY.part16.integer);
}

/**
 * @brief 6E43C | Step the entity toward the 16.16 point (x, y): speed is
 * the pixel distance in 16.16 shifted right by shift, moved along the angle
 * to the point with Entity_AccelerateAlongAngle; velX/velY get the actual displacement.
 *
 * @return the speed used
 */
u32 Entity_StepTowardPoint(struct EwramData_EntityData *entity, s32 x, s32 y, u8 shift)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    u32 dist = Sqrt(Square((oldX - x) >> 16) + Square((oldY - y) >> 16)) << 16;
    u16 angle = AngleToPoint(entity, x, y);
    u32 speed = dist >> (s8)shift;

    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    entity->velX.whole = entity->posX.whole - oldX;
    entity->velY.whole = entity->posY.whole - oldY;
    return speed;
}

/* Effect helpers 0x0806E4F8..0x0806E5FB. */
/**
 * @brief 6E4F8 | Hide (0x53C bit 5) while above the y minY, show
 * otherwise.
 */
void Entity_HideAboveY(struct EwramData_EntityData *entity, s32 minY)
{
    if ((s16)entity->posY.part16.integer < minY)
        entity->hidden = 1;
    else
        entity->hidden = 0;
}

/**
 * @brief 6E524 | Rising effect: state 0 takes the loaded graphics
 * sCommonObjectGfx (palette 7, animation 0); then rise 0.5 px per frame;
 * deleted when the animation ends.
 */
void RisingEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->scrollWithBg = 1;
            entity->drawFunc = Sprite_DrawEntity;
            entity->state++;
            break;
        }
        case 1:
            entity->posY.whole -= 0x8000;
            break;
    }
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief Increment the timer and accelerate/move with limit 0x80000,
 * then delete after timer 120. The creator supplies gravity;
 * this callback does not advance animation.
 */
void Effect_UpdateFallingFrameParticle(struct EwramData_EntityData *entity)
{
    entity->timer++;
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer > 0x78)
        EntityDelete(entity);
}

/**
 * @brief Advance animation and delete when Anim_Update returns 3.
 * Installed by the stationary animation factory.
 */
void Effect_UpdateStationaryAnimation(struct EwramData_EntityData *entity)
{
    if (Anim_Update(entity) == 3)
        EntityDelete(entity);
}

/**
 * @brief 6E5DC | Deleted when the timer timer runs out.
 */
void TimedEntity_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}

/* Probe downwards in BG1 space, returning the first offset with tile type 3. */
/**
 * @brief Probe up to 8 px downwards from a screen point and return the offset
 * of the first water surface tile (class 3), else 8. Used by water
 * enemies and the Killer Fish soul.
 */
s32 Collision_FindWaterSurfaceOffset(s16 x_, s16 y_)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 offset = 0;
    s32 i = 0;
    s16 x = x_, y = y_;
    do
    {
        s32 px = (s16)Add16(x, bg->xPos.part16.integer);
        s32 py = Add16(y, bg->yPos.part16.integer);
        s32 n = (s16)offset;
        switch (((s32 (*)(s32, s32))Tile_Classify)(px, (s16)(n + py))) /* FAKEMATCH: s32 arguments */
        {
        case 2:
            break;
        case 3:
            return n;
        }
        offset = n + 1;
    } while (i++ <= 7);
    return 8;
}

/* HitEffect_Spawn / HitEffect_SpawnAtContact (effect spawners). */

/**
 * @brief Spawn a hit effect (HitEffect_Update) at the pixel (x, y) with random
 * facing; type selects the animation (0 plain hit, 1..6 attribute index
 * from Attribute_GetHighestIndex as used by Player_ApplyContactDamage).
 */
void HitEffect_Spawn(u16 x, u16 y, u16 type)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, HitEffect_Update);

    if (effect != NULL)
    {
        effect->posX.part16.integer = x;
        effect->posY.part16.integer = y;
        effect->flipX = RandomNumberGenerator() & 1;
        effect->ext.hitEffect.type = type;
        effect->subState = type;
    }
}

/* Same as HitEffect_Spawn with signed coordinates; inlined into HitEffect_SpawnAtContact
 * (the u16 parameters of HitEffect_Spawn would give ldrh instead of ldrsh). */
static inline void SpawnHitEffect(s16 x, s16 y, u16 type)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, HitEffect_Update);

    if (child != NULL)
    {
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = RandomNumberGenerator() & 1;
        child->ext.hitEffect.type = type;
        child->subState = type;
    }
}

/**
 * @brief 6E6B4 | Spawn the hit effect of type type at the contact point
 * of attacker and entity (Hit_GetContactPoint).
 */
void HitEffect_SpawnAtContact(s32 attacker, struct EwramData_EntityData *entity, u16 type)
{
    s16 x;
    s16 y;

    Hit_GetContactPoint(attacker, entity, &x, &y);
    SpawnHitEffect(x, y, type);
}
