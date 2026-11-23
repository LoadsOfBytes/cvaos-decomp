#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern void LegionShellPart_Update(struct EwramData_EntityData *entity);
extern u8 sLegionSpriteSheet[];

/**
 * @brief 9F764 | Create a Legion part LegionShellPart_Update at the owner: layer +1,
 * graphics sLegionSpriteSheet with the owner's tiles (first word of its state
 * block) and palette (owner unk_504); kind 5 gets its own palette,
 * blending and an affine matrix from unk_502; hidden, owner in unk_4F4,
 * param_1 in unk_504, kind in unk_501, owner's attack. Returns the part;
 * when the palette allocation fails the part is deleted and the original
 * returns whatever EntityDelete leaves in r0.
 */
struct EwramData_EntityData *Legion_CreateShellPart(struct EwramData_EntityData *entity, s32 center, s32 kind)
{
    u32 *data = (u32 *)entity->ext.legion.data.value;
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)LegionShellPart_Update);
    u16 palette;
    u32 tiles;

    if (part != NULL)
    {
        part->posX.whole = entity->posX.whole;
        part->posY.whole = entity->posY.whole;
        part->spriteLayer = entity->spriteLayer + 1;
        tiles = *data;
        part->unk_510.part8.basePalette = entity->ext.legion.animPalette;
        Sprite_Attach(part, sLegionSpriteSheet, tiles, part->unk_510.part8.basePalette);
        if (kind == 5)
        {
            palette = ObjAffine_Alloc(1);
            if ((s16)palette < 0)
            {
                /* the original returns whatever EntityDelete leaves in r0 */
                return ((struct EwramData_EntityData *(*)(struct EwramData_EntityData *))EntityDelete)(part);
            }
            part->affineIndex = palette;
            part->affine = 1;
            ObjAffine_SetBiased(part->ext.legionShell.angle, 0x10000, 0x10000, part->flipX, part->flipY, part->affineIndex);
        }
        part->hidden = 1;
        part->ext.legionPart.owner = entity;
        part->ext.legionShell.center = (struct EwramData_EntityData *)center;
        part->ext.legionShell.kind = kind;
        part->attack = entity->attack;
        part->scrollWithBg = 0;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
    return part;
}
