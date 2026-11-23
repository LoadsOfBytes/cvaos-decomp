/* Object2A_LoadGraphics (object 0x2A graphics). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void Object2A_Draw(struct EwramData_EntityData *entity);

/* Graphics per variant (spawnParam1): gfx, palette, animation; unk_D is the
 * palette argument of ObjPalette_FindOrLoadVariant. */
struct Object2AGraphicsVariant {
    u8 *graphics;
    void *palette;
    void *spriteSheet;
    u8 animationIndex;
    u8 paletteIndex;
    u8 pad_E[2];
};

extern struct Object2AGraphicsVariant sObject2AVariants[];

/**
 * @brief 57774 | Load the graphics of variant spawnParam1 from sObject2AVariants
 * and draw with Object2A_Draw; 0 when no slot is free.
 */
u8 Object2A_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject2AVariants[entity->spawnParam1].graphics, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject2AVariants[entity->spawnParam1].palette,
                                                      sObject2AVariants[entity->spawnParam1].paletteIndex);
    Sprite_Attach(entity, sObject2AVariants[entity->spawnParam1].spriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Object2A_Draw;
    return 1;
}
