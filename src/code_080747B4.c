#include "code_08009A0.h"
#include "code/code_08039340.h"
extern struct EwramData_EntityData *EntityCreateInRange(s32, s32, u32 *);
extern void UneDebris_Update(struct EwramData_EntityData *);
extern u8 sUneGfx[], sUneSpriteSheet[];

/**
 * @brief Spawn a debris particle (animation 4) at (x, y) moving with
 * (velocityX, velocityY) on layer.
 */
void Une_SpawnDebris(s32 x, s32 y, s32 velocityX, s32 velocityY, s32 layer)
{
    struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)UneDebris_Update);
    if (effect != NULL)
    {
        s32 slot;
        effect->posX.whole = x;
        effect->posY.whole = y;
        effect->velX.whole = velocityX;
        effect->velY.whole = velocityY;
        effect->timer = 0;
        effect->spriteLayer = layer;
        slot = ObjGfx_Find(sUneGfx);
        Sprite_Attach(effect, sUneSpriteSheet, slot, 6);
        Anim_StartIndexed(effect, 4, 0, 1);
        Anim_Update(effect);
        effect->scrollWithBg = 1;
        effect->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
