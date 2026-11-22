/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   PeepingEye_LoadGraphics = Object25_LoadGraphics */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sPeepingEyeGfx[];
extern u8 sPeepingEyePalette[];
extern u8 sPeepingEyeSpriteSheet[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 88E08 | Same as Object25_LoadGraphics.
 */
u8 PeepingEye_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sPeepingEyeGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sPeepingEyePalette, 0);
    Sprite_Attach(entity, sPeepingEyeSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
