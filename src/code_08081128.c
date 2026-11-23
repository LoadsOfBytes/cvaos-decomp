/* Load the shared Golem graphics and the palette for its enemy variant. */
#include "code/code_08039340.h"
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern u8 sGolemGfxEnemy50[], sGolemGfx[], sGolemPalette[], sGolemSpriteSheet[];
/**
 * @brief Load the golem graphics (enemy 0x50 uses its own set) and palette variant
 * 0..3 for enemy IDs 0x35, 0x65, 0x22 and 0x50; attach the sprite set.
 * Return 0 when no graphics slot is free, else 1.
 */
u8 Golem_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics;
    if (entity->enemyId == 0x50) graphics = ObjGfx_LoadResource(sGolemGfxEnemy50, 0);
    else graphics = ObjGfx_LoadResource(sGolemGfx, 0);
    if (graphics < 0) return 0;
    if (entity->enemyId == 0x35)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 0);
    else if (entity->enemyId == 0x65)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 1);
    else if (entity->enemyId == 0x22)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 2);
    else if (entity->enemyId == 0x50)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sGolemPalette, 3);
    Sprite_Attach(entity, sGolemSpriteSheet, graphics, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
