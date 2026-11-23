/* Load the graphics 081CC014 / 08220A38 with the palette variant of
 * 0820A9A0 chosen by the type byte (0x30, 0x68, 0x48, 0x66 -> 2, 3, 0, 1;
 * other types keep basePalette) and draw with Devil_Draw; 0 when the graphics
 * are not loaded. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void Devil_Draw(struct EwramData_EntityData *entity);
extern u8 sDevilGfx[], sDevilPalette[], sDevilSpriteSheet[];

/**
 * @brief Load the Devil graphics and the palette variant of its enemy ID (0x30,
 * 0x68, ...) and attach the sprite set; return 0 when no graphics slot is
 * free, else 1.
 */
s32 Devil_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sDevilGfx, 0);
    if (graphics < 0)
        return 0;
    if (entity->enemyId == 0x30)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 2);
    else if (entity->enemyId == 0x68)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 3);
    else if (entity->enemyId == 0x48)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 0);
    else if (entity->enemyId == 0x66)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sDevilPalette, 1);
    Sprite_Attach(entity, sDevilSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Devil_Draw;
    return 1;
}
