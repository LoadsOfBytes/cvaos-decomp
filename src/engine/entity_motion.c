/*
 * Entity motion helpers: axis range and angle turning, arc velocity, distances and angles to the player, rotation, clamped / timed / circular movement, particle updates, aiming and point collision movement.
 * Built without -f2003-patch (Entity_IsPlayerWithinAxisRange / Angle_TurnTowards need it off; the rest compiles the same either way).
 */
#include "structs/ewram.h"
#include "core.h"
#include "bg_and_collision.h"
#include "entity_common.h"
#include "entity_motion.h"
#include "math_anim.h"
#include "oam.h"
#include "math_inline.h"

extern s32 ArcTan2(s32, s32);

/* from syscalls.h (not included: see the module notes) */
extern s32 Div(s32 num, s32 denom);
extern u16 Sqrt(u32 num);

/* Distance check and angle approach 0x08068688..0x08068787. Angle_TurnTowards only matches without
 * -f2003-patch (NO_2003_PATCH_SRCS in ../Makefile). */

/**
 * @brief Return 1 when the player is within the supplied unsigned
 * fixed-point limit on both axes. This is an axis-aligned test,
 * not Euclidean distance; preserve coordinate load order.
 */
s32 Entity_IsPlayerWithinAxisRange(struct EwramData_EntityData *entity, u32 range)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 dist;

    // |entity - player| per axis; written as -player + entity for the load order
    dist = -player->posX.whole + entity->posX.whole;
    if (dist < 0)
        dist = player->posX.whole - entity->posX.whole;
    if ((u32)dist > range)
        return 0;
    dist = -player->posY.whole + entity->posY.whole;
    if (dist < 0)
        dist = player->posY.whole - entity->posY.whole;
    if ((u32)dist > range)
        return 0;
    return 1;
}

/* Direction to turn from angle param_0 to angle param_1: 0 when equal,
 * 1 when going up is shorter (the angles wrap at 0x10000), else -1. */
static inline s8 AngleTurnDirection2(u16 from, u16 to)
{
    u16 down = from - to;
    u16 up = to - from;

    if (from == to)
        return 0;
    if (down > up)
        return 1;
    return -1;
}

/**
 * @brief 686CC | Turn the angle *angle by step towards target; 1 once
 * it reached target (it is then set exactly), else 0.
 */
u8 Angle_TurnTowards(u16 *angle, u16 step, u16 target)
{
    switch (AngleTurnDirection2(*angle, target))
    {
        case 0:
            goto reached;
        case 1:
            *angle += step;
            if (AngleTurnDirection2(*angle, target) == 1)
                goto turning;
            break;
        case -1:
            *angle -= step;
            if (AngleTurnDirection2(*angle, target) == -1)
                goto turning;
            break;
        default:
            goto turning;
    }
    // overshot: snap to the target
    *angle = target;
reached:
    return 1;
turning:
    return 0;
}

/* from entity_util.h (not included: see the module notes) */

static inline s32 Sq(s32 v) { return v * v; }

/**
 * @brief Unreferenced: compute the velocity for a ballistic arc from (x, y) to
 * the target with the given apex height, gravity and speed limit.
 */
s32 Entity_ComputeArcVelocity(struct EwramData_EntityData *entity, s32 x, s32 y, s32 targetX, s32 targetY, s32 height, s32 maxSpeed, u16 gravity, u16 time)
{
    s32 dist;
    s32 dx;
    s32 dy;
    u16 angle;
    u16 t;
    s16 s;
    s32 speed;
    s16 g;

    dist = Sqrt(Sq((x - targetX) >> 16) + Sq((y - targetY) >> 16)) << 16;
    angle = (u16)ArcTan2((s16)((targetX >> 16) - entity->posX.part16.integer), (s16)((targetY >> 16) - entity->posY.part16.integer));
    g = gravity;
    t = Div(height, g != 0 ? g : 1);
    dist += ((s16)t * (s16)t * g) >> 1;
    s = Sqrt(Div(dist, g != 0 ? g : 1));
    {
        s16 u = t = time + t;
        if (u > s)
            t = s * 2 - u - 1;
    }
    if ((s16)t < 0)
        speed = 0;
    else if ((g > 0 && (speed = time * g) > maxSpeed)
             || (g < 0 && (speed = time * g) < maxSpeed))
        speed = maxSpeed;
    Entity_AccelerateAlongAngle(entity, angle, speed, 0, 0, 0);
    return speed;
}

/* Move a fraction of the remaining distance towards another entity. */
static inline s32 MoveAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time)
{
    s32 step = speed + time * acceleration;
    if ((target > speed && step > target) || (target < speed && step < target))
        step = target;
    entity->posX.whole += (step >> 8) * (Sin(angle + 0x4000) >> 8);
    entity->posY.whole += (step >> 8) * (Sin(angle) >> 8);
    return step;
}

/**
 * @brief Move toward the supplied entity using distance shifted by
 * the signed interpretation of shift. Return that fixed-point step.
 * Preserve integer-pixel distance, angle conversion and helper call.
 */
s32 Entity_MoveTowardEntityFraction(struct EwramData_EntityData *entity, struct EwramData_EntityData *target, u8 shift)
{
    s32 dx = (entity->posX.whole - target->posX.whole) >> 16;
    s32 dy;
    u32 distance;
    u16 angle;

    dx *= dx;
    dy = (entity->posY.whole - target->posY.whole) >> 16;
    dy *= dy;
    distance = Sqrt(dx + dy) << 16;
    angle = (u16)ArcTan2((s16)(target->posX.part16.integer - entity->posX.part16.integer), (s16)(target->posY.part16.integer - entity->posY.part16.integer));
    distance >>= (s8)shift;
    MoveAngle(entity, angle, distance, 0, 0, 0);
    return distance;
}

/* Player-relative entity geometry and indexed slot clearing.
 * Positions use 16.16 fixed point; distance results are integer pixels
 * shifted back to fixed point. */

#define PLAYER_ENTITY (gEwramData->playerState.player)

/**
 * @brief 68938 | Clear slot (palette - 10) of sharedObjPaletteSources/sharedObjPaletteVariants for slots 0..3.
 */
void SharedObjPalette_ClearSlot(u8 palette)
{
    s8 slot = palette - 10;

    if (slot >= 0)
    {
        if (slot <= 3)
        {
            gEwramData->gameState.sharedObjPaletteSources[slot] = 0;
            gEwramData->gameState.sharedObjPaletteVariants[slot] = 0;
        }
    }
}

/**
 * @brief Return integer-pixel Euclidean distance to the actual player
 * as a fixed-point value. Preserve signed arithmetic shifts before
 * squaring; this does not select an alternate enemy target.
 */
s32 Entity_DistanceToPlayer(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;
    s32 playerX = player->posX.whole;
    s32 playerY = player->posY.whole;
    s32 dx2;
    s32 dy2;

    dx2 = (entity->posX.whole - playerX) >> 16;
    dx2 *= dx2;
    dy2 = (entity->posY.whole - playerY) >> 16;
    dy2 *= dy2;
    return Sqrt(dx2 + dy2) << 16;
}

/**
 * @brief Return absolute fixed-point horizontal distance to the player.
 */
s32 Entity_GetPlayerDistanceX(struct EwramData_EntityData *entity)
{
    s32 playerX = PLAYER_ENTITY->posX.whole;
    s32 dist = entity->posX.whole - playerX;

    if (dist < 0)
        dist = playerX - entity->posX.whole;
    return dist;
}

/**
 * @brief Return absolute fixed-point vertical distance to the player.
 */
s32 Entity_GetPlayerDistanceY(struct EwramData_EntityData *entity)
{
    s32 playerY = PLAYER_ENTITY->posY.whole;
    s32 dist = entity->posY.whole - playerY;

    if (dist < 0)
        dist = playerY - entity->posY.whole;
    return dist;
}

/**
 * @brief 689F8 | Distance between two entities (integer part, as 16.16).
 */
s32 EntityDistance(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    s32 dx2;
    s32 dy;

    dx2 = (entity->posX.whole - target->posX.whole) >> 16;
    dx2 *= dx2;
    dy = (entity->posY.whole - target->posY.whole) >> 16;
    return Sqrt(dx2 + dy * dy) << 16;
}

/* Aim at a point sixteen pixels above the player's position. */
/**
 * @brief Return the angle to a point 16 pixels above the player.
 * Preserve truncation of each pixel difference to signed 16 bits
 * before ArcTan2; this helper has matching C in its own file.
 */
u16 Entity_AngleToPointAbovePlayer(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 x = (s16)player->posX.part16.integer - (u16)entity->posX.part16.integer;
    s32 y;
    x = (s32)((u32)x << 16);
    x >>= 16;
    y = player->posY.part16.integer - (u16)entity->posY.part16.integer - 16;
    y = (s32)((u32)y << 16);
    y >>= 16;
    return ArcTan2(x, y);
}

/* Player-relative directions, distance checks, angle and screen mirrors.
 * The neighboring aim helper has matching C in this file.
 * Names describe formulas; active source use is not asserted here. */

/**
 * @brief 68A70 | Angle from entity to target.
 */
u16 EntityAngleTo(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    return (u16)ArcTan2((s16)(target->posX.part16.integer - entity->posX.part16.integer),
                   (s16)(target->posY.part16.integer - entity->posY.part16.integer));
}

/**
 * @brief 68AA0 | Mirror an angle horizontally (0x8000 - angle).
 */
u16 AngleFlipX(u16 angle)
{
    return 0x8000 - angle;
}

/**
 * @brief 68AB4 | Mirror an angle vertically (-angle).
 */
u16 AngleFlipY(u16 angle)
{
    return -angle;
}

/**
 * @brief Return 240 pixels in fixed-point units minus the supplied
 * fixed-point X coordinate.
 */
s32 Screen_MirrorFixedX(s32 x)
{
    return (240 << 16) - x;
}

/**
 * @brief Return 160 pixels in fixed-point units minus the supplied
 * fixed-point Y coordinate.
 */
s32 Screen_MirrorFixedY(s32 y)
{
    return (160 << 16) - y;
}

/**
 * @brief 68AD4 | Whether the entity lies in the rectangle at (left,
 * top) of size (width, height) (16-bit wrap-around comparison).
 */
s32 Entity_IsInRect(struct EwramData_EntityData *entity, s32 left_, s32 top_, s32 width_, s32 height_)
{
    u16 left = left_;
    u16 top = top_;
    u16 width = width_;
    u16 height = height_;

    if ((u16)(entity->posX.part16.integer - left) <= width
        && (u16)(entity->posY.part16.integer - top) <= height)
        return 1;
    return 0;
}

/**
 * @brief Return 1 when the player is right of the entity, -1 when
 * left and 0 at equal fixed-point X.
 */
s32 Entity_GetPlayerDirectionX(struct EwramData_EntityData *entity)
{
    s32 playerX = PLAYER_ENTITY->posX.whole;

    if (entity->posX.whole < playerX)
        return 1;
    if (entity->posX.whole > playerX)
        return -1;
    return 0;
}

/**
 * @brief Return 1 when the player is below the entity, -1 when
 * above and 0 at equal fixed-point Y.
 */
s32 Entity_GetPlayerDirectionY(struct EwramData_EntityData *entity)
{
    s32 playerY = PLAYER_ENTITY->posY.whole;

    if (entity->posY.whole < playerY)
        return 1;
    if (entity->posY.whole > playerY)
        return -1;
    return 0;
}

/**
 * @brief Return 1 when integer-pixel Euclidean distance to the actual
 * player, shifted to fixed point and compared unsigned, is within
 * the supplied limit. This is distinct from the axis-range test.
 */
s32 Entity_IsPlayerWithinDistance(struct EwramData_EntityData *entity, u32 distance)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;
    s32 playerX = player->posX.whole;
    s32 playerY = player->posY.whole;
    s32 dx2;
    s32 dy2;

    dx2 = (entity->posX.whole - playerX) >> 16;
    dx2 *= dx2;
    dy2 = (entity->posY.whole - playerY) >> 16;
    dy2 *= dy2;
    if ((u32)(Sqrt(dx2 + dy2) << 16) <= distance)
        return 1;
    return 0;
}

/**
 * @brief 68BC0 | Shortest turning direction from angle from to to:
 * 0 when equal, -1 or 1 otherwise.
 */
s32 AngleTurnDirection(u16 from, u16 to)
{
    u16 diff = from - to;
    u16 back = to - from;

    if (from == to)
        return 0;
    if (diff > back)
        return 1;
    return -1;
}

/* Rotate an entity position around a fixed center using 8.8 products. */
/**
 * @brief Rotate the current position around the supplied center using
 * the original products of values shifted by 8. Preserve Y-before-X
 * assignment so both calculations use the original X coordinate.
 */
void Entity_RotateAroundPoint(struct EwramData_EntityData *entity, u16 angle, s32 centerX, s32 centerY)
{
    s32 cosine = Sin(angle + 0x4000);
    s32 x = ((entity->posX.whole - centerX) >> 8) * (cosine >> 8)
          - ((entity->posY.whole - centerY) >> 8) * (Sin(angle) >> 8) + centerX;
    s32 sine = Sin(angle);
    entity->posY.whole = ((entity->posX.whole - centerX) >> 8) * (sine >> 8)
                      + ((entity->posY.whole - centerY) >> 8) * (Sin(angle + 0x4000) >> 8) + centerY;
    entity->posX.whole = x;
}

/* Entity_MoveXWithClampedStep / Entity_MoveYWithClampedStep (clamped steps). */
/**
 * @brief Add initial step plus time times acceleration to X, clamped
 * against the supplied step limit in the direction from initial
 * to limit. Return the applied step; this does not set velocity.
 */
s32 Entity_MoveXWithClampedStep(struct EwramData_EntityData *entity, s32 speed, s32 targetSpeed, s16 accel, u16 steps)
{
    s32 step = speed + steps * accel;

    if ((targetSpeed > speed && step > targetSpeed) || (targetSpeed < speed && step < targetSpeed))
        step = targetSpeed;
    entity->posX.whole += step;
    return step;
}

/**
 * @brief Add initial step plus time times acceleration to Y, clamped
 * against the supplied step limit in the direction from initial
 * to limit. Return the applied step; this does not set velocity.
 */
s32 Entity_MoveYWithClampedStep(struct EwramData_EntityData *entity, s32 speed, s32 targetSpeed, s16 accel, u16 steps)
{
    s32 step = speed + steps * accel;

    if ((targetSpeed > speed && step > targetSpeed) || (targetSpeed < speed && step < targetSpeed))
        step = targetSpeed;
    entity->posY.whole += step;
    return step;
}

/* Advance along an angle using a step bounded in its direction of change. */
/**
 * @brief Move the entity along angle by step = speed + time * acceleration,
 * clamped so it does not pass target, and return that step.
 */
s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time)
{
    s32 step = speed + time * acceleration;
    if ((target > speed && step > target) || (target < speed && step < target))
        step = target;
    entity->posX.whole += (step >> 8) * (Sin(angle + 0x4000) >> 8);
    entity->posY.whole += (step >> 8) * (Sin(angle) >> 8);
    return step;
}

/* Apply the bounded step at an angle scaled by the time parameter. */
/**
 * @brief Add a bounded step along angle times time. The step is initial
 * speed plus acceleration times time, clamped toward its limit.
 * Preserve 16-bit direction truncation and products shifted by 8.
 */
void Entity_MoveWithTimedAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time)
{
    u16 direction = angle * time;
    s32 step = speed + acceleration * time;
    if ((target > speed && step > target) || (target < speed && step < target))
        step = target;
    entity->posX.whole += (step >> 8) * (Sin(direction + 0x4000) >> 8);
    entity->posY.whole += (step >> 8) * (Sin(direction) >> 8);
}

/* Place an entity on a circle with a bounded angular rate. */
/**
 * @brief Set position around the supplied center and radius. Clamp
 * angle plus rate times time against the signed nonzero limit,
 * then multiply by time again to obtain the 16-bit direction.
 */
void Entity_SetCircularPositionAtTime(struct EwramData_EntityData *entity, s32 centerX, s32 centerY, u32 radius, s32 angle, s32 target, s32 rate, u16 time)
{
    u16 direction;
    s32 current = angle + rate * time;
    if ((target > 0 && current > target) || (target < 0 && current < target))
        current = target;
    direction = current * time;
    entity->posX.whole = centerX + (radius >> 8) * (Sin(direction + 0x4000) >> 8);
    entity->posY.whole = centerY + (radius >> 8) * (Sin(direction) >> 8);
}

/* Move along an angle and add a separate vertical displacement. */
/**
 * @brief Move along the supplied angle/speed and add vertical times
 * time to Y. Preserve the separate vertical term and shifted
 * sine products; this helper does not update velocity fields.
 */
void Entity_MoveAtAngleWithVerticalStep(struct EwramData_EntityData *entity, u16 angle, s32 speed, s16 vertical, u16 time)
{
    entity->posX.whole += (speed >> 8) * (Sin(angle + 0x4000) >> 8);
    entity->posY.whole += (speed >> 8) * (Sin(angle) >> 8) + vertical * time;
}

/* Rotate an entity position around a fixed center using 8.8 products. */
static inline void RotateAround(struct EwramData_EntityData *entity, u16 angle, s32 centerX, s32 centerY)
{
    s32 cosine = Sin(angle + 0x4000);
    s32 x = ((entity->posX.whole - centerX) >> 8) * (cosine >> 8)
          - ((entity->posY.whole - centerY) >> 8) * (Sin(angle) >> 8) + centerX;
    s32 sine = Sin(angle);
    entity->posY.whole = ((entity->posX.whole - centerX) >> 8) * (sine >> 8)
                      + ((entity->posY.whole - centerY) >> 8) * (Sin(angle + 0x4000) >> 8) + centerY;
    entity->posX.whole = x;
}

/* Place an entity on a sine path, optionally rotating it around its origin. */
/**
 * @brief Set X from travel divided by duration times time and Y from
 * one sine cycle and amplitude; optionally rotate about the origin.
 * Preserve integer division order and the original duration behavior.
 */
void Entity_SetSinePathPosition(struct EwramData_EntityData *entity, s32 originX, s32 originY,
                 u16 rotation, s32 amplitude, s32 travel, u16 duration, u16 time)
{
    u16 phase = (0x10000 / duration) * time;
    entity->posX.whole = originX + Div(travel, duration) * time;
    entity->posY.whole = originY + (amplitude >> 8) * (Sin(phase) >> 8);
    if (rotation)
        RotateAround(entity, rotation, originX, originY);
}

/* Reset all four shared palette slots. */
/**
 * @brief Clear the four shared object palette slots.
 */
void SharedObjPalette_ResetSlots(void)
{
    s32 index;
    for (index = 0; index < 4; index++)
    {
        gEwramData->gameState.sharedObjPaletteSources[index] = 0;
        gEwramData->gameState.sharedObjPaletteVariants[index] = 0;
    }
}

/* Update a generic effect with optional damping, random flips and fade. */
/**
 * @brief Generic particle update; option bits in unk_4FC: bit 0/1 damp the
 * x/y velocity, then fall (max 0x80000); unk_4FD bit 0/1 randomize
 * flipX/flipY each frame; unk_4FC bit 2 blinks out over 40 frames.
 * Deleted when the animation ends, otherwise animated.
 */
void Effect_UpdateParticle(struct EwramData_EntityData *entity)
{
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, 8);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flags & 4)
    {
        if (entity->timer < 40)
            entity->timer++;
        entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
    }
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/* Update a generic effect with optional damping, random flips and fade. */
/**
 * @brief Apply optional axis damping, movement, random flips and
 * probabilistic visibility fade. Advance animation probabilistically
 * and delete on animDone; preserve the exact RNG call conditions.
 */
void Effect_UpdateRandomlyAnimatedParticle(struct EwramData_EntityData *entity)
{
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, 8);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flags & 4)
    {
        if (entity->timer < 40 && (RandomNumberGenerator() % 128) > 32)
            entity->timer++;
        entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
    }
    if (entity->animDone)
        EntityDelete(entity);
    else if ((RandomNumberGenerator() % 128) > 32)
        Anim_Update(entity);
}

/* Update a generic effect with optional damping, random flips and fade. */
/**
 * @brief Apply optional axis damping, movement and random flips,
 * then count down to removal. Flag 4 enables visibility blinking
 * and flag 8 enables animation while the timer is nonzero.
 */
void Effect_UpdateTimedParticle(struct EwramData_EntityData *entity)
{
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, 8);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    entity->timer--;
    if (entity->ext.particle.flags & 4)
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if (entity->timer == 0)
        EntityDelete(entity);
    else if (entity->ext.particle.flags & 8)
        Anim_Update(entity);
}

/* Update a generic effect with optional damping, random flips and fade. */
/**
 * @brief Apply optional damping, movement, random flips and visibility
 * fade. On animDone free the affine slot before deleting; otherwise
 * advance animation. No active source creator is established here.
 */
void Effect_UpdateAffineParticle(struct EwramData_EntityData *entity)
{
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, 8);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flags & 4)
    {
        if (entity->timer < 40)
            entity->timer++;
        entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
    }
    if (entity->animDone)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
        Anim_Update(entity);
}

/* Update a generic effect with optional damping, random flips and fade. */
/**
 * @brief Use unk_4FE as the optional damping divisor, move and apply
 * random flips. Saturate the timer at 250, optionally hide after
 * 40 and optionally animate every other update; delete on animDone.
 */
void Effect_UpdateParticleWithDamping(struct EwramData_EntityData *entity)
{
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, entity->ext.particle.damping);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, entity->ext.particle.damping);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->timer < 250)
        entity->timer++;
    if (entity->ext.particle.flags & 4)
    {
        if (entity->timer <= 40)
            entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
        else
            entity->hidden = 1;
    }
    if (entity->animDone)
        EntityDelete(entity);
    else if (entity->ext.particle.flags & 8)
    {
        if (entity->timer & 1)
            Anim_Update(entity);
    }
    else
        Anim_Update(entity);
}

/* Set velocity towards a world point, accounting for background scroll. */
#define PIXEL_DIFF(a, b) ((a) - (b) < 0 ? -(((b) - (a)) >> 16) : ((a) - (b)) >> 16)
/**
 * @brief Set velocity toward a BG1-relative room point by dividing
 * speed times signed pixel differences by the 16-bit distance.
 * Return 1 within one speed step; retain zero-distance division.
 */
s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 dx = PIXEL_DIFF(targetX, background->xPos.whole + entity->posX.whole);
    s32 dy;
    u16 distance;
    s32 vx, vy;
    dx *= dx;
    dy = PIXEL_DIFF(targetY, background->yPos.whole + entity->posY.whole);
    dy *= dy;
    distance = Sqrt(dx + dy);
    vx = speed * PIXEL_DIFF(targetX, background->xPos.whole + entity->posX.whole) / distance;
    vy = speed * PIXEL_DIFF(targetY, background->yPos.whole + entity->posY.whole) / distance;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    if ((distance << 16) <= speed) return 1;
    return 0;
}

/* Entity_AimVelocityAtPoint (steer toward a point). */
/**
 * @brief 694B8 | Aim the velocity (speed speed) from the entity (screen
 * position + camera of background 1) at the world point (targetX,
 * targetY); 1 when the point is at most speed away.
 */
s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx;
    s32 dy;
    s32 outX;
    s32 outY;
    u16 angle;

    dx = PixelDiff(targetX, bg->xPos.whole + entity->posX.whole);
    dy = PixelDiff(targetY, bg->yPos.whole + entity->posY.whole);
    angle = (u16)ArcTan2((s16)dx, (s16)dy);
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    outX = FixedMultiply(dx, speed);
    outY = FixedMultiply(dy, speed);
    dx = PixelDiff(targetX, bg->xPos.whole + entity->posX.whole);
    dx = dx * dx;
    dy = PixelDiff(targetY, bg->yPos.whole + entity->posY.whole);
    dy = dy * dy;
    dx = Sqrt(dx + dy);
    entity->velX.whole = outX;
    entity->velY.whole = outY;
    if ((dx << 16) <= speed)
        return 1;
    return 0;
}

/* Entity_AimAccelerationAtPoint: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief Aim acceleration fields accelX/accelY at a BG1-relative
 * room point through ArcTan2 and fixed sine products. Return 1
 * when the integer-pixel distance is within the supplied magnitude.
 */
s32 Entity_AimAccelerationAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx;
    s32 dy;
    s32 outX;
    s32 outY;
    u16 angle;

    dx = PixelDiff(targetX, bg->xPos.whole + entity->posX.whole);
    dy = PixelDiff(targetY, bg->yPos.whole + entity->posY.whole);
    angle = (u16)ArcTan2((s16)dx, (s16)dy);
    dx = Sin(angle + 0x4000);
    dy = Sin(angle);
    outX = FixedMultiply(dx, speed);
    outY = FixedMultiply(dy, speed);
    dx = PixelDiff(targetX, bg->xPos.whole + entity->posX.whole);
    dx = dx * dx;
    dy = PixelDiff(targetY, bg->yPos.whole + entity->posY.whole);
    dy = dy * dy;
    dx = Sqrt(dx + dy);
    entity->accelX.whole = outX;
    entity->accelY.whole = outY;
    if ((dx << 16) <= speed)
        return 1;
    return 0;
}

/* Aim velocity at a point and immediately integrate the movement. */
/**
 * @brief Set the velocity to speed (16.16) along the line to (targetX, targetY) and
 * move one step; return 1 when the remaining distance is within speed, else 0.
 */
s32 Entity_MoveTowardPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed)
{

    s32 dx = PIXEL_DIFF(targetX, entity->posX.whole);
    s32 dy;
    u16 distance;
    s32 vx, vy;
    dx *= dx;
    dy = PIXEL_DIFF(targetY, entity->posY.whole);
    dy *= dy;
    distance = Sqrt(dx + dy);
    vx = speed * PIXEL_DIFF(targetX, entity->posX.whole) / distance;
    vy = speed * PIXEL_DIFF(targetY, entity->posY.whole) / distance;
    entity->velX.whole = vx;
    entity->velY.whole = vy;
    entity->posX.whole += vx;
    entity->posY.whole += vy;
    if ((distance << 16) <= speed) return 1;
    return 0;
}

/* Move an entity and resolve walls, ceiling and floor in background 1. */
static inline s32 PixelCoordinate(s32 origin, s32 value)
{
    if (value < 0) return origin - ((-value) >> 16);
    return origin + (value >> 16);
}
/**
 * @brief Move at limit 0x80000 and resolve point walls plus both
 * ceiling and floor regardless of vertical direction. At zero
 * X velocity query both walls; floor correction sets landing velocity.
 */
s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    EntityAccelerateAndMoveX(entity, 0x80000);
    if (entity->velX.whole <= 0)
    {
        correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            entity->posX.whole += correction << 16;
            flags = 2;
        }
    }
    if (entity->velX.whole >= 0)
    {
        correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            entity->posX.whole += correction << 16;
            flags |= 1;
        }
    }
    EntityAccelerateAndMoveY(entity, 0x80000);
    correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
    if (correction)
    {
        entity->posY.whole += correction << 16;
        flags |= 8;
    }
    correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
    {
        entity->posY.whole += correction << 16;
        entity->velY.whole = landingVelocity;
        flags |= 4;
    }
    return flags;
}

/* Move an entity and resolve walls, ceiling and floor in background 1. */
s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    if (entity->velX.whole || entity->accelX.whole)
    {
        EntityAccelerateAndMoveX(entity, speedLimit);
        if (entity->velX.whole < 0)
        {
            correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
            if (correction)
            {
                entity->posX.whole += correction << 16;
                flags = 2;
            }
        }
        if (entity->velX.whole > 0)
        {
            correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
            if (correction)
            {
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
    }
    if (entity->velY.whole || entity->accelY.whole)
    {
        EntityAccelerateAndMoveY(entity, speedLimit);
        if (entity->velY.whole < 0)
        {
            correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
            if (correction)
            {
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        if (entity->velY.whole > 0)
        {
            correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
            if (correction)
            {
                entity->posY.whole += correction << 16;
                entity->posY.part16.fraction = 0;
                entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
    }
    return flags;
}

/* Move an entity and resolve walls, ceiling and floor in background 1. */
s32 Entity_MoveWithCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit, s16 wallOffset)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    if (entity->velX.whole || entity->accelX.whole)
    {
        EntityAccelerateAndMoveX(entity, speedLimit);
        if (entity->velX.whole < 0)
        {
            correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer - wallOffset,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer - 10);
            if (correction)
            {
                entity->posX.whole += correction << 16;
                flags = 2;
            }
        }
        if (entity->velX.whole > 0)
        {
            correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer + wallOffset,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer - 10);
            if (correction)
            {
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
    }
    if (entity->velY.whole || entity->accelY.whole)
    {
        EntityAccelerateAndMoveY(entity, speedLimit);
        if (entity->velY.whole < 0)
        {
            correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
            if (correction)
            {
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        if (entity->velY.whole > 0)
        {
            correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
            if (correction)
            {
                entity->posY.whole += correction << 16;
                entity->posY.part16.fraction = 0;
                entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
    }
    return flags;
}

/* Resolve collisions and classify the upper and lower contact points. */
static inline s32 QueryCoordinate(s32 origin, s32 value)
{
    if (value < 0) return (s16)(origin - ((-value) >> 16));
    return (s16)(origin + (value >> 16));
}
/**
 * @brief Move and correct walls, optional ceiling and floor, then
 * classify upper/lower points for tile class 3. Preserve vertical
 * velocity; the second argument is unused.
 */
s32 Entity_MoveWithClassifiedPointCollision(struct EwramData_EntityData *entity, s32 unused, s32 ceilingOffset, s32 checkCeiling)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    EntityAccelerateAndMoveX(entity, 0x80000);
    if (entity->velX.whole < 0)
    {
        correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 2;
            entity->posX.whole += correction << 16;
        }
    }
    else if (entity->velX.whole > 0)
    {
        correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 1;
            entity->posX.whole += correction << 16;
        }
    }
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (checkCeiling)
    {
        correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
        if (correction)
        {
            flags |= 8;
            entity->posY.whole += correction << 16;
        }
    }
    correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
        (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
    {
        flags |= 4;
        entity->posY.whole += correction << 16;
    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}

/* Move an entity and resolve walls, ceiling and floor in background 1
 * (Entity_MoveAndCollide). The s32 block copies of the later corrections shorten the
 * life of ceilingOffset so global-alloc keeps it in sl and spills checkCeiling. */
static inline s32 NegativeCoordinate(s32 origin,s32 value)
{
    if (value < 0) return origin + ((-value) >> 16);
    return origin - (value >> 16);
}
s32 Entity_MoveAndCollide(struct EwramData_EntityData *entity, s32 sideOffset, s32 ceilingOffset, s32 checkCeiling, s32 speedLimit)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    register s32 queryX;
    EntityAccelerateAndMoveX(entity, speedLimit);
    if (entity->velX.whole <= 0)
    {
        correction = Collision_GetPushRight(NegativeCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset),
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags |= 2;
            entity->posX.whole += correction << 16;
        }
        queryX = NegativeCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset);
    }
    else
    {
        correction = Collision_GetPushLeft(PixelCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset),
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags |= 1;
            entity->posX.whole += correction << 16;
        }
        queryX = PixelCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset);
    }
    correction = Collision_GetPushDown(queryX, (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
        entity->posY.whole += correction << 16;
    EntityAccelerateAndMoveY(entity, speedLimit);
    if (checkCeiling)
    {
    { s32 c2 = (s16)Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
    if (c2)
    {
        flags |= 8;
        entity->posY.whole += c2 << 16;
    }}
    }
    if (entity->velY.whole > 0)
    {
    { s32 c2 = (s16)Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (c2)
    {
        flags |= 4;
        entity->posY.whole += c2 << 16;
    }}
    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                      QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                      (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}

/* Resolve directional collisions, apply landing velocity and classify contacts. */
/**
 * @brief Accelerate and resolve point collisions with speed limit
 * 0x80000, optionally checking the supplied ceiling offset. Return
 * wall/floor/ceiling flags and tile-class-3 flags 0x40/0x80.
 */
s32 Entity_MoveWithOptionalCeilingCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 checkCeiling)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    EntityAccelerateAndMoveX(entity, 0x80000);
    if (entity->velX.whole < 0)
    {
        correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 2;
            entity->posX.whole += correction << 16;
        }
    }
    else if (entity->velX.whole > 0)
    {
        correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 1;
            entity->posX.whole += correction << 16;
        }
    }
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (entity->velY.whole <= 0)
    {
        if (checkCeiling)
        {
            correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
            if (correction)
            {
                flags |= 8;
                entity->posY.whole += correction << 16;
            }
        }
    }
    if (entity->velY.whole >= 0)
    {
        correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags |= 4;
            entity->posY.whole += correction << 16;
            entity->velY.whole = landingVelocity;
        }
    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}

/* Move and classify collisions without correcting the resulting position. */
/**
 * @brief Move and report wall, optional ceiling, floor and class-3
 * contact flags without position correction or landing velocity
 * assignment. Preserve the unused second argument.
 */
s32 Entity_MoveAndProbePointCollision(struct EwramData_EntityData *entity, s32 unused, s32 ceilingOffset, s32 checkCeiling)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    EntityAccelerateAndMoveX(entity, 0x80000);
    if (entity->velX.whole < 0)
    {
        correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 2;

        }
    }
    else if (entity->velX.whole > 0)
    {
        correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 1;

        }
    }
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (checkCeiling)
    {
        correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
        if (correction)
        {
            flags |= 8;

        }
    }
    correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
        (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
    {
        flags |= 4;

    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}
