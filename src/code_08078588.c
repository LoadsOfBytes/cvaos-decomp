/* Falling debris released by a body segment. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void QuezlcoatlPiece_Update(struct EwramData_EntityData *);
extern u8 sQuezlcoatlWhiteDragonGfx[], sQuezlcoatlWhiteDragonSpriteSheet[];
/**
 * @brief Spawn a falling death piece (frame) at (x, y) with the owner's velocity
 * plus a random part (QuezlcoatlPiece_Update).
 */
void Quezlcoatl_SpawnDeathPiece(struct EwramData_EntityData *owner,s32 x,s32 y,s32 frame)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)QuezlcoatlPiece_Update);
    if(effect)
    {
        s32 graphics;
        
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=owner->flipX;
        effect->spriteLayer=owner->spriteLayer;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->basePalette);
        effect->animFrame=frame;
        effect->velX.whole=owner->velX.whole+(RandomNumberGenerator()&0x7FFF)-0x4000;
        effect->velY.whole=(RandomNumberGenerator()&0x7FFF)-0x24000;
        effect->accelY.whole=0x2000;
        effect->timer=RandomNumberGenerator()%4+60;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
}
