/* Create an oscillating Killer Fish effect with supplied position and velocity. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void KillerFishEffect_UpdateOscillating(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
extern u8 sKillerFishGfx[], sKillerFishSpriteSheet[];
/**
 * @brief Create a sprite effect with supplied position, velocity and frame; reject it unless its initial tile classification is 3.
 */
void KillerFish_SpawnOscillatingEffect(struct EwramData_EntityData *parent, s32 x, s32 y, s32 frame, s32 vx, s32 vy)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)KillerFishEffect_UpdateOscillating);
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->flipX = RandomNumberGenerator() % 2;
        if (RandomNumberGenerator() % 2 == 0) part->spriteLayer = parent->spriteLayer + 1;
        else part->spriteLayer = parent->spriteLayer - 1;
        graphics = ObjGfx_Find(sKillerFishGfx);
        Sprite_Attach(part, sKillerFishSpriteSheet, graphics, parent->basePalette);
        part->animFrame = frame;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
        if (Tile_Classify((s16)(background->xPos.part16.integer + part->posX.part16.integer),
                          (s16)(background->yPos.part16.integer + part->posY.part16.integer)) != 3)
            EntityDelete(part);
    }
}
