#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s8 ObjPalette_FindOrLoad(void *);
extern u8 sAltairPalette[], sAltairSpriteSheet[], sAltairGfx[];
/**
 * @brief Find or load Altair graphics and palette, attach the sprite and
 * start animation 0. Return 1 on success; delete and return 0 on the
 * existing graphics/palette failure paths.
 */
u8 Altair_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *palette = sAltairPalette;
    u8 *sprite = sAltairSpriteSheet;
    u8 *graphicsData = sAltairGfx;
    s32 graphics = ObjGfx_Find(graphicsData);
    struct EwramData_EntityData *saved = entity;
    if (graphics < 0)
    {
        graphics = ObjGfx_LoadResource(graphicsData, 0);
        if (graphics < 0) goto fail;
    }
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoad(palette);
    if (entity->unk_510.part8.basePalette == 0xFF) goto fail;
    Sprite_Attach(entity, sprite, graphics, entity->unk_510.part8.basePalette);
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
fail:
    EntityDelete(saved);
    return 0;
}
