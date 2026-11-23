/*
 * Witch: spells, orbit / trail / movement particles, cast effect and the wait state.
 */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "macros.h"
#include "code/code_080211F0.h"
#include "code_080D73B8.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* Witch homing motion and orbiting particle. */
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];
#define ABS_MOTION(v) ((v) < 0 ? -(v) : (v))

/**
 * @brief Steer toward stored room coordinates with damping and bounded
 * speed. Return nonzero when both pre-move deltas are within 0x7FFFF.
 */
s32 WitchSpell_MoveTowardStoredPoint(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
    s32 dx = entity->ext.witchSpell.targetX - (entity->posX.whole + bg->xPos.whole);
    s32 dy = entity->ext.witchSpell.targetY - (entity->posY.whole + bg->yPos.whole);
    if (dx > 0x40000) entity->accelX.whole = 0x1000;
    else if (dx < -0x40000) entity->accelX.whole = -0x1000;
    else entity->accelX.whole = 0;
    if (dy > 0x40000) entity->accelY.whole = 0x1000;
    else if (dy < -0x40000) entity->accelY.whole = -0x1000;
    else entity->accelY.whole = 0;
    EntityDampVelocityX(entity, 0x20);
    EntityDampVelocityY(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x28000);
    if (ABS_MOTION(dx) <= 0x7FFFF && ABS_MOTION(dy) <= 0x7FFFF) return 1;
    return 0;
}

/**
 * @brief Animate a particle around a scrolling center with shrinking radius
 * and changing phase; remove after its countdown. Preserve signed truncation.
 */
void WitchOrbitParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s16 radius;
    s32 radiusValue;
    s32 value;
    s32 product;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 0xD, 0, 0);
        entity->spriteLayer = 3;
        entity->timer = 0x20;
        entity->ext.witchOrbit.radius = 0x500000;
        entity->state++;
        /* fall through */
    case 1:
        bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
        entity->accelX.whole -= bg->xVel.whole;
        entity->accelY.whole -= bg->yVel.whole;
        radiusValue = entity->ext.witchOrbit.radius;
        if (radiusValue < 0) radiusValue = -((-radiusValue) >> 16) << 16;
        radius = (u32)radiusValue >> 16;
        value = Sin((s16)entity->ext.witchOrbit.angle + 0x4000);
        radius = (s16)radius;
        product = radius * value;
        value = product;
        entity->posX.whole = entity->accelX.whole + value;
        value = Sin((s16)entity->ext.witchOrbit.angle);
        product = radius * value;
        value = product;
        entity->posY.whole = entity->accelY.whole + value;
        entity->velX.whole -= 0x2800;
        entity->ext.witchOrbit.radius += entity->velX.whole;
        entity->ext.witchOrbit.angle += entity->ext.witchOrbit.spin;
        Anim_Update(entity);
        if (entity->timer != 0)
        {
            if (entity->timer == 0x10) entity->drawSkip = 1;
            entity->timer--;
        }
        else EntityDelete(entity);
        break;
    }
}

/* Witch projectile and particle helpers. */
/**
 * @brief Spawn up to two orbit particles at phases separated by 0x2000,
 * with the center and rotation direction mirrored by facing.
 */
void Witch_SpawnOrbitParticles(struct EwramData_EntityData *entity, u16 phase)
{
    s32 count = 2;
    struct EwramData_EntityData *child;
    for (;;)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)WitchOrbitParticle_Update);
        if (child != NULL)
        {
            child->accelX.part16.integer = entity->posX.part16.integer;
            child->accelY.part16.integer = entity->posY.part16.integer - 0x10;
            child->ext.witchOrbit.spin = 0xFC00;
            child->flipX = entity->flipX;
            if (child->flipX)
            {
                child->ext.witchOrbit.spin = -(u16)child->ext.witchOrbit.spin;
                child->accelX.part16.integer += 0x20;
            }
            else child->accelX.part16.integer -= 0x20;
            child->ext.witchOrbit.angle = phase;
            phase = (((u32)phase << 16) + 0x20000000) >> 16;
            if (--count == 0) break;
        }
        else break;
    }
}

/**
 * @brief Animate and move a spell trail particle, enable drawSkip after
 * the first countdown, then remove after the second. Retain state fallthrough.
 */
void WitchSpellTrailParticle_Update(struct EwramData_EntityData *entity)
{
    s32 timer;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        Anim_StartIndexed(entity, 0xE, 0, 0);
        entity->spriteLayer = 3;
        entity->timer = 6;
        entity->state++;
        /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer;
        if (timer != 0) goto countdown;
        entity->drawSkip = 1;
        entity->timer = 4;
        entity->state++;
        /* fall through */
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer;
        if (timer == 0)
        {
remove:
            EntityDelete(entity);
            return;
        }
countdown:
        entity->timer = timer - 1;
        break;
    }
    Anim_Update(entity);
}

/* Witch affine spell projectile. */
extern void EntityMoveX(struct EwramData_EntityData *);
extern void EntityMoveY(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void WitchSpell_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

/**
 * @brief Allocate an affine spell sprite, grow it, steer toward a sampled
 * target point, then continue moving. Emit trail particles and release
 * the affine slot on removal; contact applies the spell damage callback.
 */
void WitchSpell_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[entity->ext.witchSpell.ownerIndex];
    struct EwramData_EntityData *child;
    struct BgLayerState *bg;
    u32 box[4];
    u16 affine;
    u16 offset;
    s32 timer;
    s32 random;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        affine = ObjAffine_Alloc(1);
        if ((s16)affine < 0) goto remove;
        entity->affineIndex = affine;
        entity->affine = 1;
        entity->spriteLayer = 3;
        Anim_StartIndexed(entity, 4, 0, 1);
        Hit_Enable(entity, 8, WitchSpell_DamagePlayerOnContact, 0);
        HitBox_SetBodyRaw(entity, 0x2020F0F0);
        HitBox_SetAttackRaw(entity, 0x2020F0F0);
        entity->hit.flags |= 4;
        offset = 0xFFE0;
        entity->velX.whole = -0x18000;
        entity->ext.witchSpell.spin = 0xFD00;
        if (entity->flipX)
        {
            entity->velX.whole = -entity->velX.whole;
            offset = 0x20;
            entity->ext.witchSpell.spin = -entity->ext.witchSpell.spin;
        }
        {
            u16 *x = &parent->posX.part16.integer;
            s32 value = (s16)offset;
            value += *x;
            x = &entity->posX.part16.integer;
            *x = value;
        }
        entity->posY.part16.integer = (u16)parent->posY.part16.integer - 0x10;
        PlaySong(0x12A);
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->ext.witchSpell.scale <= 0xFFFF) entity->ext.witchSpell.scale += 0x2000;
        else
        {
            s32 targetX = Target_GetX(entity);
            bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
            entity->ext.witchSpell.targetX = bg->xPos.whole + targetX;
            {
                s32 targetY = Target_GetY(entity);
                bg = (struct BgLayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, bgInfo[1]));
                entity->ext.witchSpell.targetY = bg->yPos.whole + targetY - 0x100000;
            }
            entity->ext.witchSpell.delay = 0x20;
            entity->state = 2;
        }
        break;
    case 2:
        if (WitchSpell_MoveTowardStoredPoint(entity)) entity->state = 3;
        if (entity->ext.witchSpell.delay != 0) entity->ext.witchSpell.delay--;
        else entity->state = 3;
        break;
    case 3:
        EntityMoveX(entity);
        EntityMoveY(entity);
        break;
    case 4:
        ObjAffine_Free(entity->affineIndex);
remove:
        EntityDelete(entity);
        return;
    }
    Hit_CheckWithPlayer(box, entity);
    if (EntityIsOutsideScreen(entity, 3)) entity->state = 4;
    timer = entity->timer;
    timer++;
    entity->timer = timer;
    if (!(timer & 1))
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)WitchSpellTrailParticle_Update);
        if (child != NULL)
        {
            {
                s32 offsetX = RandomSigned(0x20) << 16;
                s32 x = entity->posX.whole;
                x += offsetX;
                child->posX.whole = x;
            }
            {
                s32 offsetY = RandomSigned(0x20) << 16;
                s32 y = entity->posY.whole;
                y += offsetY;
                child->posY.whole = y;
            }
            random = RandomSigned(0x8000);
            child->velX.whole = entity->velX.whole / 4 + random;
            random = RandomSigned(0x2000);
            {
                s32 speed = entity->velY.whole / 1024;
                speed -= 0x14000;
                random += speed;
                child->velY.whole = random;
            }
            child->accelX.whole = -child->velX.whole / 64;
            child->accelY.whole = -child->velY.whole / 64;
        }
    }
    ObjAffine_Set((s16)entity->ext.witchSpell.angle, entity->ext.witchSpell.scale, entity->ext.witchSpell.scale, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
    entity->ext.witchSpell.angle += entity->ext.witchSpell.spin;
}

/* Witch attached spell effect. */
/**
 * @brief Follow the owner during its casting flag, with timed drawSkip
 * transitions. Begin removal when that flag clears or the owner dies.
 */
void WitchCastEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[entity->ext.witchSpell.ownerIndex];
    s32 timer;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        entity->spriteLayer = 3;
        Anim_StartIndexed(entity, 5, 0, 1);
        entity->objMode = 1;
        entity->drawSkip = 1;
        entity->timer = 0x10;
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->drawSkip = 0;
            entity->state = 2;
        }
        break;
    case 2:
        if (!(parent->ext.witch.flags & 2))
        {
            entity->state = 3;
            entity->timer = 0x10;
        }
        break;
    case 3:
        entity->drawSkip = 1;
        timer = entity->timer;
        if (timer == 0)
        {
remove:
            EntityDelete(entity);
            return;
        }
        entity->timer = timer - 1;
        break;
    }
    Anim_Update(entity);
    if (entity->state <= 2)
    {
        if (parent->state == 3)
        {
            entity->state = 3;
            entity->timer = 0x10;
        }
        else
        {
            entity->flipX = parent->flipX;
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
        }
    }
}

/* Witch projectile and particle helpers. */
/**
 * @brief Load and move an animation-13 particle; delete on animation completion.
 */
void WitchMovementParticle_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 0xD, 0, 0);
        entity->spriteLayer = 0x11;
        entity->timer = 8;
        entity->state++;
        /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (Anim_Update(entity) == 3) EntityDelete(entity);
        break;
    }
}

/**
 * @brief Spawn a movement particle at a randomized mirrored offset, with
 * velocity and acceleration derived from the owner velocity.
 */
void Witch_SpawnMovementParticle(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)WitchMovementParticle_Update);
    u16 offset;
    s32 random;
    if (child != NULL)
    {
        offset = 0x10;
        if (entity->flipX) offset = (-(u32)offset << 16) >> 16;
        {
            s32 randomX = RandomSigned(4);
            u16 *position = &entity->posX.part16.integer;
            s32 x = (s16)offset;
            x += *position;
            x += randomX;
            position = &child->posX.part16.integer;
            *position = x;
        }
        {
            s32 y = RandomSigned(4);
            u16 *position = (u16 *)&entity->posY.part16.integer;
            y -= 4;
            y += *position;
            child->posY.part16.integer = y;
        }
        child->velX.whole = -entity->velX.whole / 2;
        random = RandomSigned(8);
        child->velY.whole = -entity->velY.whole / 4 + (random << 14);
        child->accelX.whole = -entity->velX.whole / 64;
        child->accelY.whole = -entity->velY.whole / 64;
        child->scrollWithBg = 1;
    }
}

/* Witch activation and attack selection. */
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, u16, u16);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s16 Target_GetPixelY(struct EwramData_EntityData *);

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Wait state (0): activate near the target on an animation boundary.
 * Choose charge when facing and height tests plus randomness allow it;
 * otherwise enter the spell approach state.
 */
void Witch_UpdateWait(struct EwramData_EntityData *entity)
{
    s32 width = 160;
    s32 height = 220;

    if (entity->ext.witch.flags & 4)
    {
        height += 180;
        width = 240;
    }
    if (*(u16 *)&entity->animStep == 0 && Target_IsNearEntity(entity, height, width))
    {
        s32 facing;
        s32 left;
        s32 t;
        s32 x;
        s16 *px;

        entity->ext.witch.flags |= 4;
        facing = entity->flipX;
        px = &entity->posX.part16.integer;
        t = Target_GetPixelX(entity);
        x = *px;
        left = (s16)t;
        if (x >= left)
            left = 0;
        else
            left = 1;
        if (facing == left
            && (RandomNumberGenerator() & 2)
            && ABS(Target_GetPixelY(entity) - ({ s32 t_ = entity->posY.part16.integer + 0x30; t_; })) < 0x20)
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        else
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
}
