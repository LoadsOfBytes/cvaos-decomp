/* Chaos second form: load graphics, palettes and animation data. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct ChaosData { s32 graphics, palette, palette2; };
extern u8 sChaos2Gfx[], sChaosPalette[], sChaos2PaletteSequence0[], sChaos2SpriteSheet[];
extern s8 ObjPalette_FindOrLoadVariant(void *, u8);
extern void Chaos_StartPaletteSequence(struct EwramData_EntityData *, s32);

u8 EnemyChaos2Create(struct EwramData_EntityData *entity)
{
    struct ChaosData *data = (struct ChaosData *)entity->unk_504.unk_504_entity;
    s32 graphics = ObjGfx_LoadResource(sChaos2Gfx, 0);
    if (graphics < 0)
        return 0;
    {
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sChaosPalette, 0);
        data->graphics = graphics;
        data->palette = ObjPalette_FindOrLoadVariant(sChaosPalette, 2);
        data->palette2 = ObjPalette_FindOrLoadVariant(sChaosPalette, 3);
        Chaos_StartPaletteSequence(entity, (s32)sChaos2PaletteSequence0);
        Sprite_Attach(entity, sChaos2SpriteSheet, graphics, entity->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        return 1;
    }
}
