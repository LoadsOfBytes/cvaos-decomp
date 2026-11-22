/* Werejaguar and Weretiger share graphics and select different palette rows. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *,s32);
extern u8 sWerejaguarGfx[],sWerejaguarPalette[],sWerejaguarSpriteSheet[];
u8 Werejaguar_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics=ObjGfx_LoadResource(sWerejaguarGfx,0);
    if(graphics<0) return 0;
    if(entity->unk_518.part8.enemyId==75)
        entity->unk_510.part8.basePalette=ObjPalette_FindOrLoadVariant(sWerejaguarPalette,0);
    else
        entity->unk_510.part8.basePalette=ObjPalette_FindOrLoadVariant(sWerejaguarPalette,1);
    Sprite_Attach(entity,sWerejaguarSpriteSheet,graphics,entity->unk_510.part8.basePalette);
    entity->drawFunc=(u32 *)Sprite_DrawEntity;
    return 1;
}
