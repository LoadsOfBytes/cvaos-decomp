/* Particle spawners 0x0806D5C0..0x0806D893. */
#include "code_08009A0.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];
extern u32 RandomNumberGenerator(void);
void Effect_UpdateParticle(struct EwramData_EntityData *entity);
void RisingEffect_Update(struct EwramData_EntityData *entity);
void Effect_UpdateTimedParticle(struct EwramData_EntityData *entity);

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
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)RisingEffect_Update);

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
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->spriteLayer = layer;
        entity->timer = 0;
        PARTICLE_FLAGS(entity) = 3;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(entity);
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief As Effect_SpawnParticleAnim0 with random facing and animation 0x11.
 */
void Effect_SpawnParticleAnim11(s32 x, s32 y, s32 vx, s32 vy, s32 layer)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->spriteLayer = layer;
        entity->timer = 0;
        PARTICLE_FLAGS(entity) = 3;
        entity->flipX = RandomNumberGenerator() % 2;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
        Anim_StartIndexed(entity, 0x11, 0, 0);
        Anim_Update(entity);
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief Create a countdown particle with frame 0x35, supplied position,
 * velocity, layer, palette, control flags and timer. Preserve the
 * separately stored unk_4FE byte even though this callback ignores it.
 */
void Effect_SpawnTimedFrameParticle(s32 x, s32 y, s32 vx, s32 vy, s32 layer, s32 damping, s32 flags, s32 palette, s32 duration)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateTimedParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->spriteLayer = layer;
        entity->timer = 0;
        PARTICLE_FLAGS(entity) = flags;
        PARTICLE_DAMPING(entity) = damping;
        gfx = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, palette);
        entity->animFrame = 0x35;
        entity->timer = duration;
        entity->scrollWithBg = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief 6D7F0 | Spawn a rising (velY -0x4000) Effect_UpdateParticle particle, kind
 * 4, palette slot 6, animation 4 (mode 0) or 5 with animStep = 2.
 */
void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (entity != NULL)
    {
        entity->posX.whole = x;
        entity->posY.whole = y;
        entity->velY.whole = -0x4000;
        entity->spriteLayer = layer;
        entity->timer = 0;
        PARTICLE_FLAGS(entity) = 4;
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
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
