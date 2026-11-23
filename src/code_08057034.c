/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object22_LoadGraphics = Object26_LoadGraphics */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sObject22Gfx[];
extern u8 sObject22SpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sUnk_08209FD8[];

/**
 * @brief 57034 | Same as Object26_LoadGraphics.
 */
u8 Object22_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject22Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject22SpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
