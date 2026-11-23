/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ghost_LoadGraphics = Object25_LoadGraphics */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sGhostGfx[];
extern u8 sGhostPalette[];
extern u8 sGhostSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 8A1AC | Same as Object25_LoadGraphics.
 */
u8 Ghost_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sGhostGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sGhostPalette, 0);
    Sprite_Attach(entity, sGhostSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
