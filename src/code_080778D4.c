/* Create a spark that converges on a camera independent target. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void WhiteDragonChargeParticle_Update(struct EwramData_EntityData *);
extern u8 sQuezlcoatlWhiteDragonGfx[], sQuezlcoatlWhiteDragonSpriteSheet[];
/**
 * @brief Spawn a charge particle near (x, y) that converges on the mouth over 8
 * frames before the breath (WhiteDragonChargeParticle_Update).
 */
void WhiteDragon_SpawnChargeParticle(struct EwramData_EntityData *owner,s32 x,s32 y)
{
    s32 graphics;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)WhiteDragonChargeParticle_Update);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    if(effect)
    {
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=RandomNumberGenerator()%2;
        effect->spriteLayer=owner->spriteLayer+1;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->unk_510.part8.basePalette);
        effect->animFrame=9;
        if(owner->flipX)
            effect->posX.part16.integer+=(u16)(RandomNumberGenerator()%8+16);
        else
            effect->posX.part16.integer-=(u16)(RandomNumberGenerator()%8+16);
        effect->posY.part16.integer+=RandomNumberGenerator()%32+0xFFF0;
        effect->unk_4F8.part16.unk_4F8=owner->posX.part16.integer+camera->xPos.part16.integer;
        effect->unk_4F8.part16.unk_4FA=owner->posY.part16.integer+camera->yPos.part16.integer;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
}
