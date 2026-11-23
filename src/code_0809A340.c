#include "code_08009A0.h"
#include "code/code_08039340.h"
extern struct EwramData_EntityData *EntityCreateInRange(s32, s32, u32 *);
extern void Effect_UpdateParticle(struct EwramData_EntityData *);
extern u8 sBiphronGfx[], sBiphronSpriteSheet[];

void Biphron_SpawnDebris(struct EwramData_EntityData *parent, s32 x, s32 y, s32 layer, s32 mode)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    if (effect != NULL)
    {
        s32 slot;
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->velX.whole = 0;
        effect->velY.whole = -0x10000;
        effect->accelX.whole = 0;
        effect->accelY.whole = 0;
        effect->spriteLayer = layer;
        effect->timer = 0;
        effect->ext.particle.flags = mode;
        slot = ObjGfx_Find(sBiphronGfx);
        effect->unk_510.part8.basePalette = parent->unk_510.part8.basePalette;
        Sprite_Attach(effect, sBiphronSpriteSheet, slot, effect->unk_510.part8.basePalette);
        Anim_StartIndexed(effect, 4, 0, 0);
        Anim_Update(effect);
        effect->scrollWithBg = 1;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
