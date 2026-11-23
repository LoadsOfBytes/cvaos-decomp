/* Helpers 0x08099430..0x080994AF (end of code_08098F04.s; MimicTrigger_OnHit
 * before them reads state/subState of its parent as one u16 and stays
 * asm). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

extern u8 sMimicGfx[];
extern u8 sMimicPalette[];
extern u8 sMimicSpriteSheet[];

/**
 * @brief 99430 | Load the graphics (sMimicGfx), palette from
 * sMimicPalette for (u8)spawnParam1 and start animation set sMimicSpriteSheet;
 * returns 0 when ObjGfx_LoadResource fails.
 */
s32 Mimic_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMimicGfx, 0);

    if (gfx < 0)
        return 0;

    entity->basePalette = ObjPalette_FindOrLoadVariant(sMimicPalette, (u8)entity->spawnParam1);
    Sprite_Attach(entity, sMimicSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 99480 | Substate 0: once bit 0 of 0x53D is set, start animation 1
 * and advance the substate.
 */
void Mimic_UpdatePreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0 && entity->animDone)
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->subState++;
    }
}
