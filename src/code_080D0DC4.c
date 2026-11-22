/* Randomly scattered visual effect linked back to Belmont. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BelmontPowerParticle_Update(struct EwramData_EntityData *);
extern u8 sCommonObjectGfx[],sCommonObjectSpriteSheet[];
/**
 * @brief Create a randomized frame-64 particle linked to the owner,
 * with configured movement step 1 and initial countdown 32.
 */
struct EwramData_EntityData *Belmont_CreatePowerParticle(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)BelmontPowerParticle_Update);
    if(effect)
    {
        u32 random;
        s32 graphics=ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(effect,sCommonObjectSpriteSheet,graphics,6);
        effect->animFrame=64;
        effect->unk_4F4.part8.unk_4F5=entity->unk_4F4.part8.unk_4F4;
        effect->unk_4FC.part8.unk_4FC=1;
        {
            u32 jitter=RandomNumberGenerator();
            s32 position=entity->posX.whole+0x800000;
            s32 offset=(jitter&0x3FFFFF)<<2;
            effect->posX.whole=position-offset;
        }
        {
            u32 jitter=RandomNumberGenerator();
            s32 position=entity->posY.whole+0x800000;
            s32 offset=(jitter&0x3FFFFF)<<2;
            effect->posY.whole=position-offset;
        }
        effect->spriteLayer=entity->spriteLayer-1;
        effect->scrollWithBg=1;
        effect->timer=32;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
    return effect;
}
