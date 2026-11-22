/* Effect_SpawnFadingParticle. */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Effect_UpdateParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateParticleWithDamping(struct EwramData_EntityData *entity);
extern void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Create a standard particle with X/Y damping and visibility
 * fade flags 7, palette 6 and animation 4 or 5 selected by mode.
 * The supplied velocity is retained.
 */
void Effect_SpawnFadingParticle(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
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
 * @brief Create a standard particle with supplied control flags and
 * palette, velocity and layer; mode selects animation 4 or 5.
 */
void Effect_SpawnConfiguredParticle(s32 x, s32 y, s32 velX, s32 velY, s32 spriteLayer, s32 variant, s32 flags, s32 palette)
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
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief Create the configurable-damping callback particle with supplied
 * damping byte, control flags, palette and velocity. Mode selects
 * animation 4 or 5; preserve the original field assignment order.
 */
void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticleWithDamping);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velX;
    child->velY.whole = velY;
    child->spriteLayer = layer;
    child->timer = 0;
    PARTICLE_FLAGS(child) = flags;
    PARTICLE_DAMPING(child) = damping;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, palette);
    if (variant == 0)
        Anim_StartIndexed(child, 4, 0, 0);
    else
        Anim_StartIndexed(child, 5, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
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
