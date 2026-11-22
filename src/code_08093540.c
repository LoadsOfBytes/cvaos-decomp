#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern void Entity_DrawWithOffsetCopy(struct EwramData_EntityData *);
extern u8 sCurlyGfx[], sCurlyPalette[], sCurlySpriteSheet[];

u8 Curly_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 slot = ObjGfx_LoadResource(sCurlyGfx, 0);
    u16 x, y;
    s32 room;
    if (slot < 0) return 0;
    if (entity->unk_518.part8.enemyId == 0x2B)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sCurlyPalette, 1);
    else entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sCurlyPalette, 0);
    Sprite_Attach(entity, sCurlySpriteSheet, slot, entity->unk_510.part8.basePalette);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    room = GetRoomFromMapPosition((s16)x, (s16)y);
    if (GetAreaFromMapPosition((s16)x, (s16)y) == 4 && (u32)(room - 12) <= 1)
        entity->drawFunc = (u32 *)Entity_DrawWithOffsetCopy;
    else entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
