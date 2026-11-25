/*
 * Slime: body and particles.
 */
#include "core.h"
#include "player_core.h"
#include "bg_and_collision.h"
#include "syscalls.h"
#include "graphics.h"
#include "sound.h"
#include "gameplay.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_motion.h"

extern s32 __divsi3(s32, s32);

void SlimeParticle_Update(struct EwramData_EntityData *);
void Slime_UpdateDetach(struct EwramData_EntityData *);
void Slime_DamagePlayerOnContact(struct EwramData_EntityData *);
void Slime_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/* Slime particle creation with bounded entity allocation. */

/**
 * @brief Spawn up to count slime particles with randomized position, frame
 * and facing and velocity biased by vx/vy. Stop on entity or graphics
 * allocation failure; remove a child whose graphics failed to load.
 */
void Slime_SpawnParticles(struct EwramData_EntityData *entity, s32 count, s32 vx, s32 vy)
{
    s32 index;
    struct EwramData_EntityData *child;
    index = 0;
    if (index >= count) return;
    do
    {
        child = EntityCreateInRange(0x49, 0xCF, SlimeParticle_Update);
        if (child != NULL)
        {
            if (!Entity_LoadGraphics(child, sSlimeGfx, sSlimePalette, entity->spawnParam1, sSlimeSpriteSheet))
            {
                EntityDelete(child);
                return;
            }
            child->animFrame = (RandomNumberGenerator() & 3) + 7;
            child->spriteLayer = 0x11;
            child->flipX = RandomNumberGenerator() & 1;
            child->flipY = RandomNumberGenerator() & 1;
            child->scrollWithBg = 1;
            {
                s32 random = RandomSigned(0x10);
                u16 *position = &entity->posX.part16.integer;
                s32 x = *position;
                x += random;
                child->posX.part16.integer = x;
            }
            {
                s32 random = RandomSigned(0x10);
                u16 *position = (u16 *)&entity->posY.part16.integer;
                s32 y = *position;
                y += random;
                child->posY.part16.integer = y;
            }
            child->velX.whole = vx + RandomSigned(0x20000);
            child->velY.whole = RandomSigned(0x10000) + vy;
            child->accelY.whole = 0x2000;
        }
        else return;
        index++;
    } while (index < count);
}

/* Slime terrain query and turn-speed setup. */

/**
 * @brief Query upward terrain correction and filter the resulting collision
 * flags. Return the signed correction for accepted terrain, otherwise zero;
 * the exact material interpretation of the tested flag combinations is unresolved.
 */
s16 Slime_GetFloorCorrection(s32 x, s32 y)
{
    u16 height = Collision_GetPushUp(x, y);
    s32 flags = gUnk_03002CB0.collisionProbeFlags;
    s32 valid = 0;
    s32 type;
    s32 result;
    if ((flags & 2) ? 1 : (flags & 0xC0))
    {
        if (flags & 8)
        {
            if (flags & 1) type = 2;
            else type = 1;
        }
        else type = 0;
        if (type != 1) valid = 1;
    }
    if (!valid) result = 0;
    else result = (s16)height;
    return result;
}

/* Set up a Slime surface turn from the masked angle distance.
 * The explicit register lifetimes reproduce agbcc allocation; a match does
 * not establish the original source spelling. Keep the signed rounding
 * below: it implements division by 16 with truncation toward zero. */

void Slime_BeginSurfaceTurn(struct EwramData_EntityData *entity, u8 surface, s32 bias, u16 step)
{
    u16 angle = entity->ext.slime.angle;
    register s32 mask asm("r3") = 0x7FFF; /* FAKEMATCH: pinned register */
    u16 target = sSlimeSurfaceTurnTargets[surface];
    s32 previous = entity->ext.slime.speed;
    s32 velocity;
    register s32 current asm("r1"); /* FAKEMATCH: pinned register */

    entity->ext.slime.turnSurface = surface;
    current = mask;
    current &= angle;
    mask &= target;
    if (current > mask) entity->ext.slime.speed = __divsi3(current - mask, 4);
    else entity->ext.slime.speed = __divsi3(mask - current, 4);
    if (step == 0)
    {
        register s32 loaded asm("r0") = entity->ext.slime.speed; /* FAKEMATCH: pinned register */
        velocity = loaded;
        if (velocity < 0) loaded += 15;
        entity->ext.slime.turnRate = loaded >> 4;
    }
    else
    {
        entity->ext.slime.turnRate = step;
        velocity = entity->ext.slime.speed;
    }
    entity->ext.slime.speed = velocity + bias;
    if (previous < 0) entity->ext.slime.speed = -entity->ext.slime.speed;
    entity->subState = 1;
}

/* Slime terrain support points and angle interpolation. */
#define SLIME_FIXED_INTEGER(v) ((u32)((v) < 0 ? -((-(v)) >> 16) << 16 : (v)) >> 16)

/**
 * @brief Probe center and two support points on the current surface, adjust
 * position and interpolate the tangent angle. Return nonzero if the
 * center probe loses support; retain the signed/halfword coordinate arithmetic.
 */
s32 Slime_UpdateSurfaceSupport(struct EwramData_EntityData *entity, u16 step)
{
    u16 angle = entity->ext.slime.angle;
    u16 roomX;
    u16 roomY;
    s32 lostGround = 0;
    u16 dx;
    u16 dy;
    u16 normalX;
    u16 normalY;
    s32 height;
    s32 centerX;
    s32 centerY;
    s32 tangentX;
    s32 tangentY;
    s32 perpendicularX;
    s32 perpendicularY;
    s32 rightX;
    s32 rightY;
    s32 edgeX;
    s32 firstY;
    s32 secondY;
    roomX = (u16)GetEntityRoomXPositionInteger(entity);
    roomY = (u16)GetEntityRoomYPositionInteger(entity);
    dx = SLIME_FIXED_INTEGER(6 * Sin((s16)angle + 0x4000));
    dy = SLIME_FIXED_INTEGER(6 * Sin((s16)angle));
    angle = (((u32)angle << 16) + 0x40000000) >> 16;
    normalX = SLIME_FIXED_INTEGER(6 * Sin((s16)angle + 0x4000));
    normalY = SLIME_FIXED_INTEGER(6 * Sin((s16)angle));
    if (entity->ext.slime.speed <= 0)
    {
        dx = -(u32)((u32)dx << 16) >> 16;
        dy = -(u32)((u32)dy << 16) >> 16;
    }
    centerX = (s16)roomX;
    tangentX = (s16)dx;
    rightX = centerX + tangentX;
    perpendicularX = (s16)normalX;
    entity->ext.slime.frontSupportX = (u32)((perpendicularX + rightX) << 16) >> 16;
    centerY = (s16)roomY;
    tangentY = (s16)dy;
    rightY = centerY + tangentY;
    perpendicularY = (s16)normalY;
    entity->ext.slime.frontSupportY = (u32)((perpendicularY + rightY) << 16) >> 16;
    entity->ext.slime.backSupportX = perpendicularX + (centerX - tangentX);
    entity->ext.slime.backSupportY = perpendicularY + (centerY - tangentY);
    switch (entity->stage)
    {
    case 0:
        {
            s32 centerHeight = Slime_GetFloorCorrection(centerX, centerY + 0xB);
            if (centerHeight != 0)
            {
                s32 value = (u16)entity->posY.part16.integer;
                value += 6;
                entity->posY.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = Slime_GetFloorCorrection(entity->ext.slime.frontSupportX, entity->ext.slime.frontSupportY + 6);
        if (height != 0) entity->ext.slime.frontSupportY = (u32)((height + (u16)entity->ext.slime.frontSupportY) << 16) >> 16;
        height = Slime_GetFloorCorrection(entity->ext.slime.backSupportX, entity->ext.slime.backSupportY + 6);
        if (height != 0) entity->ext.slime.backSupportY = (u32)((height + (u16)entity->ext.slime.backSupportY) << 16) >> 16;
        break;
    case 2:
        {
            s32 centerHeight = (s16)Collision_GetPushDown(centerX, centerY - 0xB);
            if (centerHeight != 0)
            {
                s32 value = (u16)entity->posY.part16.integer;
                value -= 6;
                entity->posY.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Collision_GetPushDown(entity->ext.slime.frontSupportX, entity->ext.slime.frontSupportY - 6);
        if (height != 0) entity->ext.slime.frontSupportY = (u32)((height + (u16)entity->ext.slime.frontSupportY) << 16) >> 16;
        height = (s16)Collision_GetPushDown(entity->ext.slime.backSupportX, entity->ext.slime.backSupportY - 6);
        if (height != 0) entity->ext.slime.backSupportY = (u32)((height + (u16)entity->ext.slime.backSupportY) << 16) >> 16;
        break;
    case 1:
        {
            s32 centerHeight = (s16)Collision_GetPushLeft(centerX + 0xB, centerY);
            if (centerHeight != 0)
            {
                s32 value = entity->posX.part16.integer;
                value += 6;
                entity->posX.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Collision_GetPushLeft(entity->ext.slime.frontSupportX + 6, entity->ext.slime.frontSupportY);
        if (height != 0) entity->ext.slime.frontSupportX = (u32)((height + (u16)entity->ext.slime.frontSupportX) << 16) >> 16;
        height = (s16)Collision_GetPushLeft(entity->ext.slime.backSupportX + 6, entity->ext.slime.backSupportY);
        if (height != 0) entity->ext.slime.backSupportX = (u32)((height + (u16)entity->ext.slime.backSupportX) << 16) >> 16;
        break;
    case 3:
        {
            s32 centerHeight = (s16)Collision_GetPushRight(centerX - 0xB, centerY);
            if (centerHeight != 0)
            {
                s32 value = entity->posX.part16.integer;
                value -= 6;
                entity->posX.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Collision_GetPushRight(entity->ext.slime.frontSupportX - 6, entity->ext.slime.frontSupportY);
        if (height != 0) entity->ext.slime.frontSupportX = (u32)((height + (u16)entity->ext.slime.frontSupportX) << 16) >> 16;
        height = (s16)Collision_GetPushRight(entity->ext.slime.backSupportX - 6, entity->ext.slime.backSupportY);
        if (height != 0) entity->ext.slime.backSupportX = (u32)((height + (u16)entity->ext.slime.backSupportX) << 16) >> 16;
        break;
    }
    if (entity->ext.slime.speed > 0)
    {
        edgeX = (s16)((u16)entity->ext.slime.frontSupportX - (u16)entity->ext.slime.backSupportX);
        {
            firstY = (u16)entity->ext.slime.frontSupportY;
            secondY = (u16)entity->ext.slime.backSupportY;
        }
    }
    else
    {
        edgeX = (s16)((u16)entity->ext.slime.backSupportX - (u16)entity->ext.slime.frontSupportX);
        {
            firstY = (u16)entity->ext.slime.backSupportY;
            secondY = (u16)entity->ext.slime.frontSupportY;
        }
    }
    firstY -= secondY;
    angle = ArcTan2(edgeX, (s16)firstY);
    Angle_TurnTowards((u16 *)&entity->ext.slime.angle, step, angle);
    return lostGround;
}

/* Slime crawling and transitions between floors, walls and ceilings. */
#define SLIME_PROBE_X ((s16)roomX + (s16)dx)
#define SLIME_PROBE_Y ((s16)roomY + (s16)dy)

/**
 * @brief Crawl state (0): react to hits, move along the current surface and
 * begin floor/wall/ceiling turns at corners. On non-floor surfaces,
 * proximity and randomness may select the detach state.
 */
void Slime_UpdateCrawl(struct EwramData_EntityData *entity)
{
    u16 angle = entity->ext.slime.angle;
    u16 roomX;
    u16 roomY;
    u16 dx;
    u16 dy;
    u16 normalX;
    u16 normalY;
    s32 lostGround;
    if (entity->ext.slime.flags & 1)
    {
        entity->ext.slime.flags = (entity->ext.slime.flags & ~1) | 2;
        Anim_StartIndexed(entity, 4, 0, 0);
        Slime_SpawnParticles(entity, 4, 0, -0x18000);
    }
    if ((entity->ext.slime.flags & 2) && (entity->animFlags & 4))
    {
        entity->ext.slime.flags &= ~2;
        Anim_StartIndexed(entity, 0, 0, 1);
    }
    switch (entity->subState)
    {
    case 0:
        entity->ext.slime.speed = 0x4000;
        if (entity->flipX) entity->ext.slime.speed = -entity->ext.slime.speed;
        if ((s32)entity->ext.slime.scaleX <= 0xFFFF) entity->ext.slime.scaleX += 0x200;
        if (entity->ext.slime.turnCooldown == 0 && entity->stage != 0 && (RandomNumberGenerator() & 0xFF) <= 3 && Target_IsNearEntity(entity, 0x78, 0xB4))
        {
            entity->state = 1;
            entity->subState = 0;
            return;
        }
        break;
    case 1:
        {
            s32 trig = Sin((s16)angle + 0x4000);
            s32 speed = entity->ext.slime.speed;
            trig >>= 8;
            speed >>= 8;
            trig *= speed;
            entity->velX.whole = trig;
        }
        {
            s32 trig = Sin((s16)angle);
            s32 speed = entity->ext.slime.speed;
            trig >>= 8;
            speed >>= 8;
            trig *= speed;
            entity->velY.whole = trig;
        }
        EntityMoveX(entity);
        EntityMoveY(entity);
        angle = sSlimeSurfaceTurnTargets[entity->ext.slime.turnSurface];
        if ((s32)entity->ext.slime.scaleX > 0x8000) entity->ext.slime.scaleX -= 0x200;
        if (Angle_TurnTowards((u16 *)&entity->ext.slime.angle, entity->ext.slime.turnRate, angle))
        {
            entity->subState = 0;
            entity->stage = entity->ext.slime.turnSurface;
            entity->ext.slime.turnCooldown = 0x18;
        }
        return;
    }
    if (entity->ext.slime.turnCooldown != 0)
    {
        entity->ext.slime.turnCooldown--;
    }
    EntityMoveX(entity);
    EntityMoveY(entity);
    {
        s32 trig = Sin((s16)angle + 0x4000);
        s32 speed = entity->ext.slime.speed;
        trig >>= 8;
        speed >>= 8;
        trig *= speed;
        entity->velX.whole = trig;
    }
    {
        s32 trig = Sin((s16)angle);
        s32 speed = entity->ext.slime.speed;
        trig >>= 8;
        speed >>= 8;
        trig *= speed;
        entity->velY.whole = trig;
    }
    roomX = GetEntityRoomXPositionInteger(entity);
    roomY = GetEntityRoomYPositionInteger(entity);
    dx = SLIME_FIXED_INTEGER(9 * Sin((s16)angle + 0x4000));
    dy = SLIME_FIXED_INTEGER(9 * Sin((s16)angle));
    if (entity->ext.slime.speed <= 0)
    {
        dx = -(u32)((u32)dx << 16) >> 16;
        dy = -(u32)((u32)dy << 16) >> 16;
    }
    angle = sSlimeSurfaceTurnTargets[entity->stage] - 0x4000;
    normalX = SLIME_FIXED_INTEGER(3 * Sin((s16)angle + 0x4000));
    normalY = SLIME_FIXED_INTEGER(3 * Sin((s16)angle));
    dx = (u32)(((s16)dx + (s16)normalX) << 16) >> 16;
    dy = (u32)(((s16)dy + (s16)normalY) << 16) >> 16;
    lostGround = Slime_UpdateSurfaceSupport(entity, 0x100);
    switch (entity->stage)
    {
    case 0:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 3, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 1, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Collision_GetPushLeft(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 1, 0, 0);
            }
            else if ((s16)Collision_GetPushRight(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 3, 0, 0);
        }
        break;
    case 2:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 1, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 3, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Collision_GetPushRight(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 3, 0, 0);
            }
            else if ((s16)Collision_GetPushLeft(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 0, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 2, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if ((s16)Collision_GetPushDown(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 2, 0, 0);
            }
            else if (Slime_GetFloorCorrection(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 0, 0, 0);
        }
        break;
    case 3:
        if (lostGround)
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0) Slime_BeginSurfaceTurn(entity, 2, 0x2000, 0);
            else Slime_BeginSurfaceTurn(entity, 0, 0x2000, 0);
        }
        else
        {
            if (entity->ext.slime.turnCooldown != 0) break;
            if (entity->ext.slime.speed > 0)
            {
                if (Slime_GetFloorCorrection(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 0, 0, 0);
            }
            else if ((s16)Collision_GetPushDown(SLIME_PROBE_X, SLIME_PROBE_Y)) Slime_BeginSurfaceTurn(entity, 2, 0, 0);
        }
        break;
    }
}

/* Slime setup, dispatch and shrinking death animation. */

/**
 * @brief Detach state (1): animate departure from wall or ceiling, move
 * under gravity, emit particles on surface contact, then align and
 * recover to crawling.
 */
void Slime_UpdateDetach(struct EwramData_EntityData *entity)
{
    s32 baseSpeed = 0x8000;
    s32 flags;
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        /* fall through */
    case 1:
        if (!(entity->animFlags & 4)) break;
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->ext.slime.flags |= 4;
        switch (entity->stage)
        {
        case 2:
            entity->ext.slime.speed = 0;
            entity->velY.whole = 0;
            break;
        case 1:
            {
                u32 random = RandomNumberGenerator();
                s32 mask = 0x1FFF;
                mask &= random;
                entity->ext.slime.speed = -(baseSpeed + mask);
            }
            entity->velY.whole = RandomSigned(0x8000) - 0x10000;
            break;
        case 3:
            {
                u32 random = RandomNumberGenerator();
                s32 mask = 0x1FFF;
                mask &= random;
                entity->ext.slime.speed = baseSpeed + mask;
            }
            entity->velY.whole = RandomSigned(0x8000) - 0x10000;
            break;
        }
        PlaySong(0x164);
        entity->accelY.whole = 0x2000;
        entity->subState = 2;
        /* fall through */
    case 2:
        flags = Entity_Walk(entity, entity->ext.slime.speed, sSlimeWalkProfile, 0x22);
        if (flags & 8)
        {
            Slime_SpawnParticles(entity, 3, 0, -0x20000);
            entity->stage = 0;
            entity->subState = 3;
            break;
        }
        if (flags & 1)
        {
            Slime_SpawnParticles(entity, 3, 0x10000, -0x18000);
            entity->stage = 3;
            entity->subState = 3;
            break;
        }
        if (flags & 2)
        {
            Slime_SpawnParticles(entity, 3, -0x10000, -0x18000);
            entity->stage = 1;
            entity->subState = 3;
        }
        break;
    case 3:
        PlaySong(0x163);
        entity->ext.slime.angle = sSlimeSurfaceTurnTargets[entity->stage];
        entity->ext.slime.flags &= ~4;
        Slime_UpdateSurfaceSupport(entity, 0x4000);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 4;
        /* fall through */
    case 4:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->subState = 0;
            entity->ext.slime.turnCooldown = 0;
        }
        break;
    }
}

/* Slime setup, dispatch and shrinking death animation. */
/**
 * @brief Death state (2): land if detached, shrink while moving along the
 * surface normal and emit particles. Return nonzero when the scale
 * is exhausted; the dispatcher owns affine-slot cleanup.
 */
s32 Slime_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 angle = entity->ext.slime.angle + 0x4000;
    s32 timer;
    s32 dx;
    s32 dy;
    s32 scale;
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x70);
        Anim_StartIndexed(entity, 0, 0, 1);
        if (entity->ext.slime.flags & 4)
        {
            entity->accelY.whole = 0x2000;
            entity->ext.slime.angle = 0;
        }
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->ext.slime.flags & 4)
        {
            if (Entity_Walk(entity, entity->ext.slime.speed, sSlimeWalkProfile, 0x22) & 8)
            {
                entity->ext.slime.flags &= ~4;
                entity->ext.slime.speed = 0;
                {
                    s32 correction = (0x10000 - entity->ext.slime.scaleY) >> 8;
                    s32 factor = 0x500;
                    factor *= correction;
                    entity->posY.whole += factor;
                }
            }
        }
        if (entity->ext.slime.scaleY > 0)
        {
            {
                s32 value = Sin(angle + 0x4000) >> 8;
                scale = 0x12;
                dx = value;
                dx *= scale;
                value = Sin(angle) >> 8;
                dy = value;
                dy *= scale;
                {
                    s32 x = entity->posX.whole;
                    x += dx;
                    entity->posX.whole = x;
                }
                {
                    s32 y = entity->posY.whole;
                    y += dy;
                    entity->posY.whole = y;
                }
            }
            timer = entity->timer;
            timer++;
            entity->timer = timer;
            if (!(timer & 3)) Slime_SpawnParticles(entity, 1, 0, -0x20000);
            if (!(entity->timer & 0xF)) PlaySong(0x165);
            entity->ext.slime.scaleY -= 0x400;
        }
        else return 1;
        break;
    }
    return 0;
}

void EnemySlimeCreate(struct EwramData_EntityData *entity)
{
    u16 affine;
    s32 x;
    s32 height;
    if (!Entity_LoadGraphics(entity, sSlimeGfx, sSlimePalette, entity->spawnParam1, sSlimeSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affine;
    entity->affine = 1;
    entity->ext.slime.angle = 0;
    entity->ext.slime.scaleX = 0x10000;
    entity->ext.slime.scaleY = 0x10000;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x11;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        ObjAffine_Set(entity->ext.slime.angle, entity->ext.slime.scaleX, entity->ext.slime.scaleY, entity->flipX, 0, entity->affineIndex);
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Slime_DamagePlayerOnContact, Slime_OnHit);
    HitBox_SetBodyRaw(entity, 0x1010F8F8);
    HitBox_SetAttackRaw(entity, 0x1010F8F8);
    if (entity->spawnParam0 == 0) Entity_SnapToGround(entity);
    else
    {
        for (;;)
        {
            x = (s16)GetEntityRoomXPositionInteger(entity);
            height = (s16)Collision_GetPushDown(x, (s16)GetEntityRoomYPositionInteger(entity));
            if (height == 0)
            {
                entity->posY.part16.integer = (u16)entity->posY.part16.integer - 8;
            }
            else break;
        }
        entity->posY.part16.integer = height + ((u16)entity->posY.part16.integer + 5);
        entity->stage = 2;
        entity->ext.slime.angle = 0x8000;
    }
    ObjAffine_Set(entity->ext.slime.angle, entity->ext.slime.scaleX, entity->ext.slime.scaleY, entity->flipX, 0, entity->affineIndex);
    entity->scrollWithBg = 1;
    entity->state = 0;
}

void EnemySlimeUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: Slime_UpdateCrawl(entity); break;
    case 1: Slime_UpdateDetach(entity); break;
    case 2:
        if (Slime_UpdateDeath(entity))
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    ObjAffine_Set(entity->ext.slime.angle, entity->ext.slime.scaleX, entity->ext.slime.scaleY, entity->flipX, 0, entity->affineIndex);
    Enemy_CheckHitPlayer(entity);
    Anim_Update(entity);
}

/* Slime helpers 0x080C13DC..0x080C148F. */

/**
 * @brief C13DC | Player_ApplyContactDamage with (attack, 0x40, 0, 0) at the entity
 * position.
 */
void Slime_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0x40;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Mark the hit-reaction flag and apply enemy damage; on defeat drop
 * rewards, restore the palette and enter death state 2.
 */
void Slime_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    entity->ext.slime.flags |= 1;
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief Accelerate and move the particle with speed cap 0x40000; request
 * deletion outside the expanded screen rectangle.
 */
void SlimeParticle_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x40000);
    if (!Entity_IsInRect(entity, -0x20, -0x20, 0x130, 0xE0))
        entity->deleteRequested = 1;
}
