/* Object29_LoadGraphics (object 0x29 graphics). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void Object29_Draw(struct EwramData_EntityData *entity);

/* Graphics per variant (spawnParam1): gfx, palette, animation; unk_D is the
 * palette argument of ObjPalette_FindOrLoadVariant. */
struct Object29GraphicsVariant {
    u8 *graphics;
    void *palette;
    void *spriteSheet;
    u8 animationIndex;
    u8 paletteIndex;
    u8 pad_E[2];
};

extern struct Object29GraphicsVariant sObject29Variants[];

/**
 * @brief 573B0 | Load the graphics of variant spawnParam1 from sObject29Variants
 * and draw with Object29_Draw; 0 when no slot is free.
 */
u8 Object29_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject29Variants[entity->spawnParam1].graphics, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sObject29Variants[entity->spawnParam1].palette,
                                                      sObject29Variants[entity->spawnParam1].paletteIndex);
    Sprite_Attach(entity, sObject29Variants[entity->spawnParam1].spriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Object29_Draw;
    return 1;
}
