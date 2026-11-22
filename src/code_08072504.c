/* Create a falling debris sprite, selecting its frame and vertical flip. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void MudmanPiece_Update(struct EwramData_EntityData *);
extern u8 sMudmanGfx[], sMudmanSpriteSheet[];
/**
 * @brief Spawn a mud piece at (x, y) moving with (vx, vy) under gravity, frame 21
 * or 18 by direction, flipped vertically when moving down.
 */
void Mudman_SpawnPiece(struct EwramData_EntityData *parent, s32 x, s32 y, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)MudmanPiece_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->accelY.whole = 0x2800;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sMudmanGfx);
        Sprite_Attach(part, sMudmanSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        if (vx > 0) part->animFrame = 21;
        else part->animFrame = 18;
        if (vy > 0) part->flipY = 1;
        else part->flipY = 0;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        part->timer = 0;
    }
}
