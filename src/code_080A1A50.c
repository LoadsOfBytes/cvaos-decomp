/* Spawn seven fragments with the owner's palette and horizontal motion. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void GladiatorFragment_Update(struct EwramData_EntityData *);
extern u8 sGladiatorGfx[], sGladiatorSpriteSheet[];
/**
 * @brief Burst into seven fragments at (x, y) with the velocity of entity plus a
 * random part (GladiatorFragment_Update).
 */
void Gladiator_SpawnFragments(struct EwramData_EntityData *entity, s32 x, s32 y)
{
    s32 i;
    for (i = 0; i <= 6; i++)
    {
        struct EwramData_EntityData *effect = EntityCreateInRange(0x49, 0xCF, (u32 *)GladiatorFragment_Update);
        if (effect != NULL)
        {
            effect->posX.whole = x;
            effect->posY.whole = y;
            effect->velX.whole = entity->velX.whole + (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            effect->velY.whole = -(RandomNumberGenerator() & 0xFFFF) - 0x4000;
            effect->accelY.whole = 0x1000;
            effect->spriteLayer = entity->spriteLayer;
            Sprite_Attach(effect, sGladiatorSpriteSheet, ObjGfx_Find(sGladiatorGfx), entity->unk_510.part8.basePalette);
            Anim_StartIndexed(effect, (u16)(i + 5), 0, 0);
            effect->flipX = entity->flipX;
            effect->scrollWithBg = 1;
            effect->drawFunc = (u32 *)Sprite_DrawEntity;
            effect->timer = (RandomNumberGenerator() % 4) + 0x28;
        }
    }
}
