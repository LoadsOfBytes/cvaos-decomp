/* Release six pieces with distinct frames and randomized horizontal velocities. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void GorgonPiece_Update(struct EwramData_EntityData *);
extern u8 sGorgonGfx[], sGorgonSpriteSheet[];
/**
 * @brief Spawn the six body pieces (frames 17..22) that drop away when the Gorgon
 * dies, with a random horizontal speed and lifetime 40..55 frames.
 */
void Gorgon_SpawnDeathPieces(struct EwramData_EntityData *parent)
{
    s32 index;
    for (index = 5; index >= 0; index--)
    {
        struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)GorgonPiece_Update);
        if (part)
        {
            s32 graphics;
            s32 lifetime;
            part->posX.whole = parent->posX.whole;
            part->posY.whole = parent->posY.whole;
            part->velX.whole = (RandomNumberGenerator() & 0xFFFF) - 0x8000;
            part->velY.whole = 0;
            part->accelY.whole = 0x1400;
            part->spriteLayer = parent->spriteLayer;
            graphics = ObjGfx_Find(sGorgonGfx);
            Sprite_Attach(part, sGorgonSpriteSheet, graphics, parent->basePalette);
            part->animFrame = index + 17;
            part->flipX = parent->flipX;
            part->scrollWithBg = 1;
            part->drawFunc = (u32 *)Sprite_DrawEntity;
            lifetime = RandomNumberGenerator() & 15;
            part->timer = lifetime + 40;
        }
    }
}
