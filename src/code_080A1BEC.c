/* Load the shared fragment graphics (081CC0AC / 08229104) with the two
 * palette variants of 0820AF98 into unk_504 / unk_505, the second one also
 * as the palette (unk_510); 0 when the graphics are not loaded. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sGladiatorGfx[], sGladiatorSpriteSheet[], sGladiatorPalette[];

s32 Gladiator_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGladiatorGfx, 0);
    if (graphics < 0)
        return 0;
    entity->ext.gladiator.palette = ObjPalette_FindOrLoadVariant(sGladiatorPalette, 0);
    entity->unk_510.part8.basePalette = entity->ext.gladiator.palette1 = ObjPalette_FindOrLoadVariant(sGladiatorPalette, 1);
    Sprite_Attach(entity, sGladiatorSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
