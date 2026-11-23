/* Graphics setup. */
#include "code/code_08039340.h"
#include "balore_entity.h"
#include "structs/ewram.h"
extern u8 ObjPalette_FindOrLoadVariant(void *, s32);
extern u8 sBaloreGfx[], sBalorePalette[], sBaloreSpriteSheet[];

/**
 * @brief B45C0 | Graphics setup: palette slot from ObjGfx_LoadResource (0 on
 * failure), sprite set sBalorePalette variant 7 (0 with the bestiaryPreview mode
 * flag) into unk_4F8 byte 0 and basePalette, slot into unk_4FB, registers
 * sBaloreSpriteSheet with Sprite_Attach, draw callback Sprite_DrawEntity, scrollWithBg.
 * Same shape as Object21_LoadGraphics.
 */
u8 Balore_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 slot = ObjGfx_LoadResource(sBaloreGfx, 0);

    if (slot < 0)
        return 0;
    if (!gEwramData->gameState.bestiaryPreview)
        entity->ext.balore.palette = ObjPalette_FindOrLoadVariant(sBalorePalette, 7);
    else
        entity->ext.balore.palette = ObjPalette_FindOrLoadVariant(sBalorePalette, 0);
    entity->ext.balore.gfx = slot;
    entity->basePalette = entity->ext.balore.palette;
    Sprite_Attach(entity, sBaloreSpriteSheet, slot, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    entity->scrollWithBg = 1;
    return 1;
}
