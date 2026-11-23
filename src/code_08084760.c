/* Headhunter transformation effects: falling parts (84760/8489C) and two
 * affine bursts in four mirrored corners that launch the rolling waves
 * (84B20/84C7C) or the lightning (84DB8/84F14). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sHeadhunterFrameAnim;
extern u8 sHeadhunterSpriteSheet[];
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Effect_SpawnParticleAnim13(s32, s32, s32, s32, s32, s32);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void EntityAccelerateAndMoveY(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Headhunter_SpawnLightningBurstFlicker(struct EwramData_EntityData *, s32, s32);
extern void Headhunter_SpawnWave(struct EwramData_EntityData *, s32, s32, s32);
extern void Headhunter_SpawnLightningBolt(struct EwramData_EntityData *, s32, s32);
extern void Headhunter_SpawnOrbitingParticle(struct EwramData_EntityData *, struct EwramData_EntityData *, s32, s32, s32);
void HeadhunterFallingPart_Update(struct EwramData_EntityData *entity);
void HeadhunterWaveBurst_Update(struct EwramData_EntityData *entity);
void HeadhunterLightningBurst_Update(struct EwramData_EntityData *entity);
/* bit 0 of the owner's unk_4FE ends its effects (TODO: meaning) */
#define OWNER_DONE(o) (o->ext.headhunter.flags & 1)
#define FRAME(e) (e->ext.headhunterEffect.frame)
#define KIND(e) (*(s8 *)&e->ext.headhunterChild.kind) /* also the burst corner 0-3 */
#define CORNER(e) KIND(e)
#define OFFSET_X(e) ((e)->velX.whole)
#define OFFSET_Y(e) ((e)->velY.whole)
#define SCALE_X(e) ((e)->velX.whole)
#define SCALE_Y(e) ((e)->velY.whole)
#define ROTATION(e) ((e)->accelX.whole)

/* Create a falling part of kind 0-2 at the owner + (x, y). */
/**
 * @brief Create an affine falling part of kind 0..2 at an owner-relative
 * offset with its form palette and frame. Preserve the short-cast
 * affine failure check and the kind-dependent starting countdown.
 */
void Headhunter_SpawnFallingPart(struct EwramData_EntityData *owner, s32 x, s32 y, s32 kind)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterFallingPart_Update);
    s32 graphics;
    u16 slot;
    if (part == NULL) return;
    OFFSET_X(part) = x;
    OFFSET_Y(part) = y;
    part->flipX = owner->flipX;
    part->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    switch (kind)
    {
    case 0:
        part->basePalette = owner->ext.headhunter.palette0;
        FRAME(part) = 1;
        part->timer = 8;
        break;
    case 1:
        part->basePalette = owner->ext.headhunter.palette1;
        FRAME(part) = 0x6C;
        part->timer = 8;
        break;
    case 2:
        part->basePalette = owner->ext.headhunter.palette2;
        FRAME(part) = 0x4F;
        part->timer = 60;
        break;
    }
    Sprite_Attach(part, sHeadhunterSpriteSheet, graphics, part->basePalette);
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(part);
        return;
    }
    part->affineIndex = slot;
    part->affine = 1;
    ObjAffine_SetBiased(0, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
    ENTITY_OWNER(part) = owner;
    KIND(part) = kind;
    part->scrollWithBg = 0;
    part->drawFunc = (u32 *)Sprite_DrawEntity;
}

/* Falling part: stick to the owner shedding sparks, then fall, bounce on
 * the floor (y 0xB7) inside the room x range and blink out. */
/**
 * @brief Follow the owner while emitting sparks, then fall, rotate and
 * blink within the room bounds. Kind 2 advances the owner on release;
 * other kinds do so on expiry. Free the affine slot on removal.
 */
void HeadhunterFallingPart_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if (entity->subState == 0)
    {
        entity->posX.whole = owner->posX.whole + OFFSET_X(entity);
        entity->posY.whole = owner->posY.whole + OFFSET_Y(entity);
        if ((entity->timer & 7) == 0)
        {
            s32 x = entity->posX.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            s32 y = entity->posY.whole + (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            s32 vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 vy = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            Effect_SpawnParticleAnim13(x, y, vx, vy, entity->spriteLayer - 1, 2);
        }
        ObjAffine_SetBiased(0, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((s8)entity->timer-- <= 0)
        {
            entity->subState++;
            entity->timer = 60;
            entity->velX.whole = entity->flipX ? 0x10000 : -0x10000;
            ROTATION(entity) = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0x1800;
            entity->scrollWithBg = 1;
            if (KIND(entity) == 2) owner->subState++;
        }
    }
    else
    {
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        EntityMoveX(entity);
        EntityAccelerateAndMoveY(entity, 0x40000);
        if (entity->posY.whole + camera->yPos.whole > 0xB70000)
        {
            entity->posY.whole = 0xB70000 - camera->yPos.whole;
            entity->velY.whole = 0x10000;
            EntityDampVelocityX(entity, 8);
        }
        if (entity->posX.whole + camera->xPos.whole < 0x670000)
            entity->posX.whole = 0x670000 - camera->xPos.whole;
        if (entity->posX.whole + camera->xPos.whole > 0x1980000)
            entity->posX.whole = 0x1980000 - camera->xPos.whole;
        if ((entity->velX.whole < 0 ? -entity->velX.whole : entity->velX.whole) > 0x2000)
        {
            if (entity->flipX) ROTATION(entity) += 0x400;
            else ROTATION(entity) -= 0x400;
        }
        ObjAffine_SetBiased(ROTATION(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x10000, 0x80000, 0x40000, entity->spriteLayer - 1);
        if ((s8)entity->timer-- <= 0)
        {
            if (KIND(entity) != 2) owner->subState++;
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}

/* Create a burst of the wave attack in one of four mirrored corners. */
/**
 * @brief Create one mirrored corner of the affine wave-attack burst,
 * with countdown 0x6C, owner graphics and stored contact strength.
 * Remove the entity if affine allocation fails.
 */
void Headhunter_SpawnWaveBurst(struct EwramData_EntityData *owner, s32 x, s32 y, s32 corner)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterWaveBurst_Update);
    s32 graphics;
    u16 slot;
    if (effect == NULL) return;
    effect->posX.whole = owner->posX.whole + x;
    effect->posY.whole = owner->posY.whole + y;
    switch (corner)
    {
    case 0:
        effect->flipX = 0;
        effect->flipY = 0;
        break;
    case 1:
        effect->flipX = 1;
        effect->flipY = 0;
        break;
    case 2:
        effect->flipX = 0;
        effect->flipY = 1;
        break;
    case 3:
        effect->flipX = 1;
        effect->flipY = 1;
        break;
    }
    CORNER(effect) = corner;
    effect->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    effect->ext.headhunterEffect.graphics = graphics;
    effect->basePalette = owner->ext.headhunter.palette3;
    Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
    FRAME(effect) = 0x59;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(effect);
        return;
    }
    effect->affineIndex = slot;
    effect->affine = 1;
    effect->objMode = 1;
    SCALE_X(effect) = 0x10000;
    SCALE_Y(effect) = 0;
    ObjAffine_SetBiased(0, 0x10000, 0, effect->flipX, effect->flipY, effect->affineIndex);
    effect->timer = 0x6C;
    effect->attack = owner->attack;
    ENTITY_OWNER(effect) = owner;
    effect->scrollWithBg = 1;
    effect->drawFunc = (u32 *)Sprite_DrawEntity;
}

/* Wave burst: corner 0 launches two rolling waves at 58 frames and eight
 * falling particles at 70. */
/**
 * @brief Grow and shrink the affine burst. Corner 0 creates two moving
 * waves at countdown 58 and eight orbiting particles at 70. Free
 * the affine slot on timeout or owner effect-cancel bit 0.
 */
void HeadhunterWaveBurst_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    s32 i;
    if (entity->timer > 67)
    {
        if (SCALE_Y(entity) <= 0xFFFF) SCALE_Y(entity) += 0xC00;
    }
    else if (entity->timer == 58)
    {
        if (CORNER(entity) == 0)
        {
            Headhunter_SpawnWave(owner, 0, -0x140000, 0);
            Headhunter_SpawnWave(owner, 0, 0x100000, 1);
            PlaySong(0x7A);
        }
    }
    else if (entity->timer <= 24)
    {
        if (SCALE_Y(entity) > 0xBFF) SCALE_Y(entity) -= 0xC00;
    }
    if (CORNER(entity) == 0 && entity->timer == 70)
    {
        for (i = 0; i < 8; i++)
            Headhunter_SpawnOrbitingParticle(entity, owner, 0, -0x20000, i << 13);
    }
    ObjAffine_SetBiased(0, SCALE_X(entity), SCALE_Y(entity), entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if ((s8)entity->timer-- <= 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (OWNER_DONE(owner))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/* Create a burst of the lightning attack in one of four mirrored corners. */
/**
 * @brief Create one mirrored corner of the affine lightning-attack
 * burst with initially zero scale and rotation. Preserve the comma
 * expression assigning the common zero and the corner.
 */
void Headhunter_SpawnLightningBurst(struct EwramData_EntityData *owner, s32 x, s32 y, s32 corner)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)HeadhunterLightningBurst_Update);
    s32 graphics;
    s32 zero;
    u16 slot;
    if (effect == NULL) return;
    effect->posX.whole = owner->posX.whole + x;
    effect->posY.whole = owner->posY.whole + y;
    switch (corner)
    {
    case 0:
        effect->flipX = 0;
        effect->flipY = 0;
        break;
    case 1:
        effect->flipX = 1;
        effect->flipY = 0;
        break;
    case 2:
        effect->flipX = 0;
        effect->flipY = 1;
        break;
    case 3:
        effect->flipX = 1;
        effect->flipY = 1;
        break;
    }
    /* the zero for the transform fields is set together with the corner */
    CORNER(effect) = (zero = 0, corner);
    effect->spriteLayer = owner->spriteLayer - 1;
    graphics = owner->ext.headhunter.graphics;
    effect->ext.headhunterEffect.graphics = graphics;
    effect->basePalette = owner->ext.headhunter.palette3;
    Sprite_Attach(effect, sHeadhunterSpriteSheet, graphics, effect->basePalette);
    FRAME(effect) = 0x59;
    if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
    {
        EntityDelete(effect);
        return;
    }
    effect->affineIndex = slot;
    effect->affine = 1;
    effect->objMode = 1;
    SCALE_X(effect) = zero;
    SCALE_Y(effect) = zero;
    ROTATION(effect) = zero;
    ObjAffine_SetBiased(0, 0, 0, effect->flipX, effect->flipY, effect->affineIndex);
    effect->attack = owner->attack;
    ENTITY_OWNER(effect) = owner;
    effect->scrollWithBg = 1;
    effect->drawFunc = (u32 *)Sprite_DrawEntity;
}

/* Lightning burst: grow, corner 0 spawns the effect at 42 and three bolts
 * at 58 frames, then shrink. */
/**
 * @brief Grow and rotate the burst; corner 0 creates its flicker at
 * timer 42 and three bolts at 58. Shrink and free the affine slot
 * on signed timer expiry or owner effect-cancel bit 0.
 */
void HeadhunterLightningBurst_Update(struct EwramData_EntityData *entity)
{
    struct SingleFrameAnimation animation = sHeadhunterFrameAnim;
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    if (entity->timer++ <= 32)
    {
        if (SCALE_X(entity) <= 0xFFFF) SCALE_X(entity) += 0x800;
        if (SCALE_Y(entity) <= 0xFFFF) SCALE_Y(entity) += 0x800;
        if (ROTATION(entity) <= 0xFFFF) ROTATION(entity) += 0x800;
    }
    else if (entity->timer == 42)
    {
        if (CORNER(entity) == 0) Headhunter_SpawnLightningBurstFlicker(owner, 0, 0);
    }
    else if (entity->timer == 58)
    {
        if (CORNER(entity) == 0)
        {
            s32 x, y;
            x = entity->posX.whole;
            y = 0x7A0000 - camera->yPos.whole;
            Headhunter_SpawnLightningBolt(owner, x, y);
            x = entity->posX.whole - 0x400000;
            y = 0x800000 - camera->yPos.whole;
            Headhunter_SpawnLightningBolt(owner, x, y);
            x = entity->posX.whole + 0x400000;
            y = 0x800000 - camera->yPos.whole;
            Headhunter_SpawnLightningBolt(owner, x, y);
        }
    }
    else if (entity->timer > 95)
    {
        if (SCALE_X(entity) > 0x9FF) SCALE_X(entity) -= 0xA00;
        if (SCALE_Y(entity) > 0x9FF) SCALE_Y(entity) -= 0xA00;
        ROTATION(entity) += 0x800;
    }
    ObjAffine_SetBiased(ROTATION(entity), SCALE_X(entity), SCALE_Y(entity), entity->flipX, entity->flipY, entity->affineIndex);
    Anim_Start(entity, (struct AnimScript *)&animation, 2, 1);
    animation.frame = FRAME(entity);
    Anim_Update(entity);
    if ((s8)entity->timer < 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (OWNER_DONE(owner))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
