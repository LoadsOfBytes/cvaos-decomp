#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern u8 sBigGolemGfx[], sBigGolemPalette[], sBigGolemSpriteSheet[];
/* Big Golem graphics: allocate tiles and two palette variants. */
u8 BigGolem_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 tiles = ObjGfx_LoadResource(sBigGolemGfx, 0);
    if (tiles < 0)
        return 0;
    entity->unk_508.part8.unk_50A = ObjPalette_FindOrLoadVariant(sBigGolemPalette, 0);
    entity->unk_508.part8.unk_50B = ObjPalette_FindOrLoadVariant(sBigGolemPalette, 1);
    entity->unk_510.part8.basePalette = entity->unk_508.part8.unk_50A;
    Sprite_Attach(entity, sBigGolemSpriteSheet, tiles, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
