/* Create a detached Killer Fish sprite using the parent's cached graphics. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void KillerFishEffect_UpdateRising(struct EwramData_EntityData *);
extern u8 sKillerFishGfx[], sKillerFishSpriteSheet[];
/**
 * @brief Create a detached sprite with parent graphics and palette at the supplied position and frame, one layer above the parent.
 */
void KillerFish_SpawnRisingSprite(struct EwramData_EntityData *parent, s32 x, s32 y, s32 frame)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)KillerFishEffect_UpdateRising);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = RandomNumberGenerator() % 2;
        part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sKillerFishGfx);
        Sprite_Attach(part, sKillerFishSpriteSheet, graphics, parent->basePalette);
        part->animFrame = frame;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
