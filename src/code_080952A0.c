/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Lilith_LoadGraphics = Object32_LoadGraphics */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u8 sLilithSuccubusGfx[];
extern u8 sLilithSuccubusPalette[];
extern u8 sLilithSuccubusSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 952A0 | Same as Object32_LoadGraphics.
 */
u8 Lilith_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sLilithSuccubusGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sLilithSuccubusPalette, 1);
    Sprite_Attach(entity, sLilithSuccubusSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
