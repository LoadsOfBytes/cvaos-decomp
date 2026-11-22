/* Short visual effect attached to Belmont's current position. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BelmontMovementEffect_Update(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[],sCommonObjectSpriteSheet[];
/**
 * @brief Create an animation-4 visual effect slightly above the owner
 * using shared effect graphics. Return it or NULL on entity allocation failure.
 */
struct EwramData_EntityData *Belmont_CreateMovementEffect(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)BelmontMovementEffect_Update);
    if(effect)
    {
        s32 graphics=ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect,sCommonObjectSpriteSheet,graphics,6);
        Anim_StartIndexed(effect,4,0,0);
        effect->posX.whole=entity->posX.whole;
        effect->posY.whole=entity->posY.whole-0x30000;
        effect->spriteLayer=entity->spriteLayer-1;
        effect->objMode=1;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
    return effect;
}
