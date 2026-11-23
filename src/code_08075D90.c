/* Defeat fragments inherit their owner's sprite palette and scatter randomly. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void HarpyDeathFeather_Update(struct EwramData_EntityData *);
extern u8 sHarpyGfx[],sHarpySpriteSheet[];
/**
 * @brief Spawn a loose feather at (x, y) with a random velocity (four on death).
 */
void Harpy_SpawnDeathFeather(struct EwramData_EntityData *owner,s32 x,s32 y)
{
    struct EwramData_EntityData *fragment=EntityCreateInRange(0x49,0xCF,(u32 *)HarpyDeathFeather_Update);
    if(fragment)
    {
        s32 zero;
        s32 graphics;
        fragment->posX.whole=x;
        fragment->posY.whole=y;
        fragment->velX.whole=(RandomNumberGenerator() & 0x3FFFF)-0x20000;
        fragment->velY.whole=(RandomNumberGenerator() & 0x3FFFF)-0x20000;
        zero=0;
        fragment->accelX.whole=zero;
        fragment->accelY.whole=zero;
        fragment->spriteLayer=owner->spriteLayer-1;
        graphics=ObjGfx_Find(sHarpyGfx);
        Sprite_Attach(fragment,sHarpySpriteSheet,graphics,owner->basePalette);
        fragment->animFrame=8;
        fragment->flipX=RandomNumberGenerator()%2;
        fragment->scrollWithBg=1;
        fragment->drawFunc=(u32 *)Sprite_DrawEntity;
        fragment->timer=zero;
    }
}
