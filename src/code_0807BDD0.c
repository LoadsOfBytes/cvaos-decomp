/* Load graphics and choose the drawing callback for the room. */
#include "code/code_08039340.h"
#include "code_08001194.h"
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern void Entity_DrawWithOffsetCopy(struct EwramData_EntityData *);
extern u8 sPersephoneGfx[], sPersephonePalette[], sPersephoneSpriteSheet[];
/**
 * @brief Load the Persephone graphics and palette; in area 4 rooms 12 / 13 draw
 * with Entity_DrawWithOffsetCopy instead of Sprite_DrawEntity. Return 0 when no
 * graphics slot is free, else 1.
 */
u8 Persephone_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sPersephoneGfx, 0);
    u16 x, y;
    s32 room;
    if (graphics < 0) return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sPersephonePalette, 0);
    Sprite_Attach(entity, sPersephoneSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    room = GetRoomFromMapPosition((s16)x, (s16)y);
    if (GetAreaFromMapPosition((s16)x, (s16)y) == 4 && (u32)(room - 12) <= 1)
        entity->drawFunc = (u32 *)Entity_DrawWithOffsetCopy;
    else
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
