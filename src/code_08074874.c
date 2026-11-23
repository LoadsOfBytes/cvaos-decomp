/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Une_LoadGraphics = Object25_LoadGraphics */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sUneGfx[];
extern u8 sUnePalette[];
extern u8 sUneSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 74874 | Same as Object25_LoadGraphics.
 */
u8 Une_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sUneGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnePalette, 0);
    Sprite_Attach(entity, sUneSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
