/* Load the graphics 081190C0 / 082258FC with the two palette variants of
 * 0820AC4C (unk_504 / unk_505, the second also as unk_510) and record the
 * graphics slot and both variants in the shared data block at unk_4F8;
 * 0 when the graphics are not loaded. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sLegionGfx[], sLegionSpriteSheet[], sLegionObject25Palette[];

/**
 * @brief Load the Legion graphics and its two palettes (unk_504 / unk_505) and
 * store the tile and palette slots in the state block; return 0 when no
 * graphics slot is free.
 */
s32 Legion_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics;
    s32 *data = (s32 *)entity->ext.legion.data.ptr;
    graphics = ObjGfx_LoadResource(sLegionGfx, 0);
    if (graphics < 0)
        return 0;
    entity->ext.legion.animPalette = ObjPalette_FindOrLoadVariant(sLegionObject25Palette, 0);
    entity->unk_510.part8.basePalette = entity->ext.legion.bodyPalette = ObjPalette_FindOrLoadVariant(sLegionObject25Palette, 1);
    data[0] = graphics;
    data[8] = (s8)entity->ext.legion.animPalette;
    data[9] = (s8)entity->ext.legion.bodyPalette;
    Sprite_Attach(entity, sLegionSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
