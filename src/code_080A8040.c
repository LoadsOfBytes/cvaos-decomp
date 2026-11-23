#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Effect_UpdateParticle(struct EwramData_EntityData *);
extern u8 sSkyFishGfx[], sSkyFishSpriteSheet[];
/* Create a moving animated effect. Position arguments use 16.16 fixed point. */
void SkyFish_SpawnTrail(struct EwramData_EntityData *parent, s32 x, s32 y, s32 priority, s32 param)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    if (child != NULL)
    {
        s32 tiles;
        child->posX.whole = x;
        child->posY.whole = y;
        child->velX.whole = 0;
        child->velY.whole = 0x4000;
        child->accelX.whole = 0;
        child->accelY.whole = 0;
        child->ext.particle.damping = 0x20;
        child->spriteLayer = priority;
        child->timer = 0;
        child->ext.particle.flags = param;
        if (parent->flipX)
            child->velX.whole = 0x20000;
        else
            child->velX.whole = -0x20000;
        tiles = ObjGfx_Find(sSkyFishGfx);
        child->basePalette = parent->basePalette;
        Sprite_Attach(child, sSkyFishSpriteSheet, tiles, child->basePalette);
        Anim_StartIndexed(child, 1, 0, 0);
        Anim_Update(child);
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Companion effect with upward acceleration. */
void SkyFish_SpawnRisingTrail(struct EwramData_EntityData *parent, s32 x, s32 y, s32 priority, s32 param)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    if (child != NULL)
    {
        s32 tiles;
        child->posX.whole = x;
        child->posY.whole = y;
        child->velX.whole = 0;
        child->velY.whole = 0;
        child->accelX.whole = 0;
        child->accelY.whole = -0x1000;
        child->ext.particle.damping = 0x18;
        child->spriteLayer = priority;
        child->timer = 0;
        child->ext.particle.flags = param;
        tiles = ObjGfx_Find(sSkyFishGfx);
        child->basePalette = parent->basePalette;
        Sprite_Attach(child, sSkyFishSpriteSheet, tiles, child->basePalette);
        Anim_StartIndexed(child, 1, 0, 0);
        Anim_Update(child);
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
