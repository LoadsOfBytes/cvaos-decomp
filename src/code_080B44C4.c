#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sBaloreSpriteSheet[];
extern const u8 sBaloreEyeAnimOffsets[];

/**
 * @brief B44C4 | Balore part (created by EnemyBaloreCreate, owner in
 * unk_4FC): graphics sBaloreSpriteSheet with the owner's tiles and palette,
 * palette slot 0x1C; shown while the owner is not in state 3, hidden for
 * 16 frames when the owner's unk_502 bit 0 is set, deleted with bit 1;
 * follows the owner's position and blending, animation 9 +
 * sBaloreEyeAnimOffsets[owner unk_4F6].
 */
void BaloreBodyPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.balorePart.body;

    if (owner->ext.balore.flags & 2)
        entity->state = 4;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)owner->ext.balore.gfx, owner->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0x1C;
        entity->state = 1;
    case 1:
        if (owner->state == 3)
        {
            entity->hidden = 0;
            entity->state = 3;
            break;
        }
        entity->hidden = 1;
        if (owner->ext.balore.flags & 1)
        {
            entity->hidden = 0;
            entity->timer = 0x10;
            entity->state = 2;
        }
        break;
    case 2:
        if (entity->timer != 0)
            entity->timer--;
        else
            entity->state = 1;
        break;
    case 4:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    entity->palette = owner->palette;
    Anim_StartIndexed(entity, sBaloreEyeAnimOffsets[(s8)owner->ext.balore.eyeAnim] + 9, 2, 0);
    Anim_Update(entity);
}
