#include "code_08009A0.h"
#include "gate_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Gate_Draw(struct EwramData_EntityData *);
extern u8 sGateGfx[], sGatePalette[], sGateSpriteSheet[], sGatePaletteAnim0[], sGatePaletteAnim1[];

/**
 * @brief 4D8F0 | Graphics setup of object 0: layer 0x11, facing left when
 * right of x 0x78, sprite sGateGfx / palette sGatePalette /
 * sGateSpriteSheet (Entity_LoadGraphics; palette slot 5 in unk_510 when it fails),
 * palette slot kept in unk_4FF; frame 0 with palette data sGatePaletteAnim0 when
 * opened is 0, otherwise frame 5 with sGatePaletteAnim1 (PaletteAnim_Start); two
 * affine slots (ObjAffine_Alloc(1)) in unk_4FC / unk_4FD, drawn by
 * Gate_Draw.
 */
void Gate_SetupGraphics(struct EwramData_EntityData *entity, s32 opened)
{
    u8 pal;

    entity->spriteLayer = 0x11;
    entity->flipX = (s16)entity->posX.part16.integer > 0x78 ? 1 : 0;
    if (!Entity_LoadGraphics(entity, sGateGfx, sGatePalette, 0, sGateSpriteSheet))
        entity->unk_510.part8.basePalette = 5;
    GATE_PALETTE(entity) = entity->unk_510.part8.basePalette;
    pal = GATE_PALETTE(entity);
    entity->roomActive = 1;
    if (opened == 0)
    {
        entity->animFrame = opened;
        PaletteAnim_Start(sGatePalette, sGatePaletteAnim0, (s8)pal);
    }
    else
    {
        entity->animFrame = 5;
        PaletteAnim_Start(sGatePalette, sGatePaletteAnim1, (s8)pal);
    }
    GATE_AFFINE_0(entity) = ObjAffine_Alloc(1);
    GATE_AFFINE_1(entity) = ObjAffine_Alloc(1);
    entity->affineIndex = GATE_AFFINE_0(entity);
    entity->affine = 1;
    entity->drawFunc = (u32 *)Gate_Draw;
}
