#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *graphics, void *sprite, u8 palette, void *paletteData);

/**
 * @brief B0064 | Affine-rotated companion sprite of the parent (unk_4F8):
 * allocates an affine slot, follows the parent while its frame animScriptIndex is 4
 * and its pose (unk_4F8 - 4) is 0 or 1, swinging on a rotating arc and
 * growing over 7 frames; freed with its slot when the parent id changes.
 */
void MinotaurSwingTrail_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 slot;
    s32 angle;
    s32 rot;
    s32 dx, dy;

    if (entity->state != 0 && ENTITY_INDEX(parent) != ENTITY_PARENT_INDEX(entity))
        entity->state = 2;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, (void *)0x081E41A4, (void *)0x0820B7B0, 0, (void *)0x0823AE10))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->scrollWithBg = 1;
        Anim_StartIndexed(entity, 9, 0, 1);
        Anim_Update(entity);
        entity->spriteLayer = parent->spriteLayer;
        entity->state++;
    case 1:
        if (parent->animScriptIndex != 4)
            goto hide;
        if ((u8)(parent->ext.minotaur.animStep - 4) > 1)
        {
            entity->timer = 0;
            entity->hidden = 1;
            entity->ext.minotaurTrail.angle = 0;
            return;
        }
        if (entity->timer > 6)
        {
        hide:
            entity->hidden = 1;
            return;
        }
        entity->timer++;
        entity->hidden = 0;
        entity->subState = parent->ext.minotaur.animStep - 4;
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x10;
        entity->ext.minotaurTrail.angle += -0x400;
        rot = entity->ext.minotaurTrail.angle;
        angle = (rot - 0x6000) & 0xFFFF;
        dx = Sin(angle + 0x4000) * 56;
        dy = Sin(angle) << 6;
        if (entity->flipX)
        {
            dx = -dx;
            rot = -rot;
        }
        entity->posX.whole += dx;
        entity->posY.whole += dy;
        break;
    case 2:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(rot, 0x14000 - (entity->timer << 11), 0x12000, entity->flipX, 1, entity->affineIndex);
}
