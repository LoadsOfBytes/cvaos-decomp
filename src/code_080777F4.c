/* Short lived sparks emitted by the linked body. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void WhiteDragonSpark_Update(struct EwramData_EntityData *);
extern u8 sQuezlcoatlWhiteDragonGfx[], sQuezlcoatlWhiteDragonSpriteSheet[];
/**
 * @brief Spawn a short spark at (x, y) on layer, randomly flipped
 * (WhiteDragonSpark_Update, 9 frames).
 */
void WhiteDragon_SpawnSpark(struct EwramData_EntityData *owner,s32 x,s32 y,s32 layer)
{
    s32 graphics;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,(u32 *)WhiteDragonSpark_Update);
    if(effect)
    {
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=RandomNumberGenerator()%2;
        effect->spriteLayer=layer;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->basePalette);
        effect->animFrame=9;
        if(owner->flipX)
            effect->posX.part16.integer+=RandomNumberGenerator()%6;
        else
            effect->posX.part16.integer-=RandomNumberGenerator()%6;
        effect->posY.part16.integer+=RandomNumberGenerator()%6-3;
        effect->scrollWithBg=1;
        effect->drawFunc=(u32 *)Sprite_DrawEntity;
    }
}


