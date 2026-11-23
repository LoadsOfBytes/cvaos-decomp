/* Arachne effects inherit the owner's palette, facing and layer. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void ArachneEffect_Update(struct EwramData_EntityData *);
extern u8 sArachneGfx[],sArachneSpriteSheet[];
/**
 * @brief Spawn an effect at (x, y) with the Arachne graphics: animation, lifetime
 * frames, or until the animation ends when the owner timer is 128.
 */
void Arachne_SpawnEffect(struct EwramData_EntityData *owner,s32 x,s32 y,s32 animation,s32 lifetime)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)ArachneEffect_Update);
    if(effect)
    {
        s32 graphics;
        effect->posX.whole=x; effect->posY.whole=y;
        effect->flipX=owner->flipX;
        effect->spriteLayer=owner->spriteLayer;
        graphics=ObjGfx_Find(sArachneGfx);
        Sprite_Attach(effect,sArachneSpriteSheet,graphics,owner->basePalette);
        if(owner->timer!=128) Anim_StartIndexed(effect,animation,0,1);
        else Anim_StartIndexed(effect,animation,0,0);
        effect->timer=lifetime;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
}
