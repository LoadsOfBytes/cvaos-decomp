/* Gorgon and Catoblepas share graphics and select different palette variants. */
#include "code/code_08039340.h"
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern u8 sGorgonGfx[], sGorgonPalette[], sGorgonSpriteSheet[];
/**
 * @brief Load the Gorgon graphics with palette variant 0 for type 0x33, else 1;
 * return 0 when no graphics slot is free, else 1.
 */
u8 Gorgon_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGorgonGfx, 0);
    if (graphics < 0) return 0;
    if (entity->enemyId == 0x33)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGorgonPalette, 0);
    else
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGorgonPalette, 1);
    Sprite_Attach(entity, sGorgonSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
