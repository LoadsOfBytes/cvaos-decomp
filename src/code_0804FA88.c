#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "syscalls.h"

extern void DebrisPiece_Update(struct EwramData_EntityData *entity);

/**
 * @brief 4FA88 | Burst of 3 x 3 debris pieces (DebrisPiece_Update) at the owner's
 * position, rows dy - 16, dy, dy + 16, with random frame 0x40..0x42 and
 * random flips; stops a row when no entity slot is free.
 */
void Debris_SpawnBurst(struct EwramData_EntityData *owner, s32 dy)
{
    s32 graphics = ObjGfx_Find((void *)0x081C15F4);
    s32 y;
    s32 n;
    struct EwramData_EntityData *piece;
    s8 palette = 6;

    for (y = -0x10; y <= 0x10; y += 0x10)
    {
        for (n = 3; n > 0; n--)
        {
            piece = EntityCreateInRange(0x49, 0xCF, (u32 *)DebrisPiece_Update);
            if (piece == NULL)
                break;
            Sprite_Attach(piece, (void *)0x0820ED60, graphics, palette);
            piece->animFrame = Mod(RandomNumberGenerator() >> 4, 3) + 0x40;
            piece->posX.part16.integer = owner->posX.part16.integer;
            piece->posY.part16.integer = owner->posY.part16.integer + dy + y;
            piece->flipX = (RandomNumberGenerator() >> 7) & 1;
            piece->flipY = (RandomNumberGenerator() >> 7) & 1;
            piece->spriteLayer = 0x15;
            piece->scrollWithBg = 1;
            piece->drawFunc = (u32 *)Sprite_DrawEntity;
        }
    }
}
