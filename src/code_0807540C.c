/* Burst of seven fragments (frames 0x24-0x2A) from the owner position with
 * random velocities, gravity 0x1800, the owner palette and facing, and a
 * lifetime of 40-43 frames (WingedSkeletonPiece_Update). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void WingedSkeletonPiece_Update(struct EwramData_EntityData *);
extern u8 sWingedSkeletonGfx[], sWingedSkeletonSpriteSheet[];

void WingedSkeleton_SpawnDeathPieces(struct EwramData_EntityData *owner)
{
    s32 i;
    for (i = 0; i <= 6; i++)
    {
        struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)WingedSkeletonPiece_Update);
        if (effect != NULL)
        {
            effect->posX.whole = owner->posX.whole;
            effect->posY.whole = owner->posY.whole;
            effect->velX.whole = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
            effect->velY.whole = -(RandomNumberGenerator() & 0xFFFF) - 0x4000;
            effect->accelY.whole = 0x1800;
            effect->spriteLayer = owner->spriteLayer;
            Sprite_Attach(effect, sWingedSkeletonSpriteSheet, ObjGfx_Find(sWingedSkeletonGfx), owner->unk_510.part8.basePalette);
            effect->animFrame = i + 0x24;
            effect->flipX = owner->flipX;
            effect->scrollWithBg = 1;
            effect->drawFunc = (u32 *)Sprite_DrawEntity;
            effect->timer = RandomNumberGenerator() % 4 + 0x28;
        }
    }
}
