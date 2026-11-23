/* Chaos second form: effect and spinning projectile creation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct ChaosEffectGfx { s32 tiles; s32 palette; };
extern u8 sChaos2SpriteSheet[];
extern void Chaos2Effect_Update(struct EwramData_EntityData *entity);
extern void Chaos2SpinningDebris_Update(struct EwramData_EntityData *entity);
extern u32 RandomNumberGenerator(void);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *entity, s32 angle, s32 speed);

/* Copy graphics from the parent's owner and attach an animated effect. */
void Chaos2_SpawnEffect(struct EwramData_EntityData *parent, s32 frame)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2Effect_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaosChild.link->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        Anim_StartIndexed(effect, (u16)frame, 2, 0);
        Anim_Update(effect);
        effect->enemyId = parent->enemyId;
        effect->ext.chaosChild.link = parent;
        effect->scrollWithBg = 0;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Attach an animated effect one layer above the parent. */
void Chaos2_SpawnEffectAbove(struct EwramData_EntityData *parent, s32 frame)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2Effect_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer + 1;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        Anim_StartIndexed(effect, (u16)frame, 2, 0);
        Anim_Update(effect);
        effect->enemyId = parent->enemyId;
        effect->ext.chaosChild.link = parent;
        effect->scrollWithBg = 0;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Create a spinning projectile; delete it if no affine matrix is free. */
void Chaos2_SpawnSpinningDebris(struct EwramData_EntityData *parent, s32 angle, s32 speed)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2SpinningDebris_Update);
    struct ChaosEffectGfx *gfx = (struct ChaosEffectGfx *)parent->ext.chaos.data.ptr;
    if (effect != NULL)
    {
        s32 tiles;
        u16 matrix;
        effect->posX.part16.integer = parent->posX.part16.integer;
        effect->posY.part16.integer = parent->posY.part16.integer;
        effect->spriteLayer = parent->spriteLayer;
        tiles = gfx->tiles;
        effect->basePalette = gfx->palette;
        Sprite_Attach(effect, sChaos2SpriteSheet, tiles, effect->basePalette);
        effect->animFrame = 0x4A;
        matrix = ObjAffine_Alloc(1);
        if ((s16)matrix < 0)
        {
            EntityDelete(effect);
        }
        else
        {
            effect->affineIndex = matrix;
            effect->affine = 1;
            ObjAffine_Set(0, 0x10000, 0x10000, effect->flipX, effect->flipY, effect->affineIndex);
            effect->enemyId = parent->enemyId;
            effect->ext.chaosChild.link = parent;
            effect->scrollWithBg = 0;
            effect->accelX.whole = (RandomNumberGenerator() & 0x3FF) - 0x200;
            EntitySetVelocityFromAngle(effect, angle, speed);
            effect->drawFunc = (u32 *)Sprite_DrawEntity;
        }
    }
}

