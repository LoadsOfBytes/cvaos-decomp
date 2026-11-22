/* Load the graphics 081CC004 / 0821F240 with the palette variant of
 * 0820A8F8 chosen by the type byte (0x3C, 0x67, 0x69 -> 0, 1, 2; other
 * types keep unk_510); 0 when the graphics are not loaded. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sGreatArmorGfx[], sGreatArmorPalette[], sGreatArmorSpriteSheet[];

s32 GreatArmor_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGreatArmorGfx, 0);
    if (graphics < 0)
        return 0;
    if (entity->unk_518.part8.enemyId == 0x3C)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sGreatArmorPalette, 0);
    else if (entity->unk_518.part8.enemyId == 0x67)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sGreatArmorPalette, 1);
    else if (entity->unk_518.part8.enemyId == 0x69)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sGreatArmorPalette, 2);
    Sprite_Attach(entity, sGreatArmorSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
