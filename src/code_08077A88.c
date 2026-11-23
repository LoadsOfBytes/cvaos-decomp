/* Falling debris released by a body segment. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void WhiteDragonPiece_Update(struct EwramData_EntityData *);
extern u8 sQuezlcoatlWhiteDragonGfx[], sQuezlcoatlWhiteDragonSpriteSheet[];
/**
 * @brief Spawn a falling death piece (frame) at (x, y) facing like owner
 * (WhiteDragonPiece_Update).
 */
void WhiteDragon_SpawnDeathPiece(struct EwramData_EntityData *owner,s32 x,s32 y,s32 frame)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)WhiteDragonPiece_Update);
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
        effect->velX.whole=(RandomNumberGenerator()&0x7FFF)-0x4000;
        effect->velY.whole=0;
        effect->accelY.whole=0x1000;
        effect->timer=44;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
}
