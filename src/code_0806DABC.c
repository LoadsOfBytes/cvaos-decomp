/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Effect_SpawnFadingParticleCopy = Effect_SpawnFadingParticle */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Effect_UpdateRandomlyAnimatedParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateFallingFrameParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateStationaryAnimation(struct EwramData_EntityData *entity);
extern void Effect_UpdateTimedParticle(struct EwramData_EntityData *entity);
extern void TimedEntity_Update(struct EwramData_EntityData *entity);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Same as Effect_SpawnFadingParticle.
 */
void Effect_SpawnFadingParticleCopy(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateRandomlyAnimatedParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = layer;
    child->timer = 0;
    PARTICLE_FLAGS(child) = 7;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief Spawn an Effect_UpdateParticle particle (animation 0xC) with
 * velocity, layer, kind and palette.
 */
void Effect_SpawnParticleAnim12(s32 x, s32 y, s32 velX, s32 velY, s32 spriteLayer, s32 flags, s32 palette)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = spriteLayer;
    child->timer = 0;
    PARTICLE_FLAGS(child) = flags;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, palette);
    Anim_StartIndexed(child, 0xC, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief Create a frame-0x35 particle with supplied velocity and
 * gravity 0x2800, palette 6 and the fixed-lifetime falling callback.
 */
void Effect_SpawnFallingFrameParticle(s32 x, s32 y, s32 velX, s32 velY, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateFallingFrameParticle);
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
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief Create an animation-0xA effect at the supplied position and
 * layer with palette 6. Its callback advances animation until
 * completion; this creator performs no initial animation update.
 */
void Effect_SpawnStationaryAnimation(s32 x, s32 y, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateStationaryAnimation);
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
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 6DCC4 | Spawn a falling piece (Effect_UpdateTimedParticle) at (x, y)
 * with velocity (velX, velY), gravity 0x2000, on layer layer,
 * timer 200, palette 6; animFrame 0x40 / 0x41 / 0x42 for piece 0 / 1 / other.
 */
void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTimedParticle);
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
    PARTICLE_FLAGS(child) = 0;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 6);
    if (piece == 0)
        child->animFrame = 0x40;
    else if (piece == 1)
        child->animFrame = 0x41;
    else
        child->animFrame = 0x42;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief Spawn an Effect_UpdateParticle particle (animation 0xD, random
 * facing, kind 3, palette 6 + n) with velocity on the given layer.
 */
void Effect_SpawnParticleAnim13(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 paletteOffset)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->flipX = RandomNumberGenerator() % 2;
    child->spriteLayer = layer;
    PARTICLE_FLAGS(child) = 3;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, paletteOffset + 6);
    Anim_StartIndexed(child, 0xD, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 6DE04 | Spawn a TimedEntity_Update entity at (x, y) on
 * layer layer with flags bits 2 and 1 and 0x53D bit 1 set (param_2 ..
 * param_4 are not used).
 */
void Effect_SpawnTimedEntity(s32 x, s32 y, s32 param_2, s32 param_3, s32 param_4, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)TimedEntity_Update);

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->spriteLayer = layer;
    child->hit.flags |= 4;
    child->hit.flags |= 2;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}
