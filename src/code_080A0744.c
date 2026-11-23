/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KyomaDemon_LoadGraphics = Object25_LoadGraphics */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sKyomaDemonGfx[];
extern u8 sKyomaDemonPalette[];
extern u8 sKyomaDemonSpriteSheet[];
extern void KyomaDemon_Draw(struct EwramData_EntityData* entity);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief A0744 | Same as Object25_LoadGraphics.
 */
u8 KyomaDemon_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sKyomaDemonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sKyomaDemonPalette, 0);
    Sprite_Attach(entity, sKyomaDemonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)KyomaDemon_Draw;
    return 1;
}
