/* Harpy and Siren share graphics but select different palette variants. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *,s32);
extern u8 sHarpyGfx[],sHarpyPalette[],sHarpySpriteSheet[];
u8 Harpy_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics=ObjGfx_LoadResource(sHarpyGfx,0);
    if(graphics<0) return 0;
    if(entity->unk_518.part8.enemyId==56)
        entity->unk_510.part8.basePalette=ObjPalette_FindOrLoadVariant(sHarpyPalette,0);
    else
        entity->unk_510.part8.basePalette=ObjPalette_FindOrLoadVariant(sHarpyPalette,1);
    Sprite_Attach(entity,sHarpySpriteSheet,graphics,entity->unk_510.part8.basePalette);
    entity->drawFunc=(u32 *)Sprite_DrawEntity;
    return 1;
}
