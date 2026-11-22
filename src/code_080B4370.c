#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sBaloreSpriteSheet[];
extern const u8 sBalorePartAnimOffsets[];

void BalorePart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4FC.unk_4FC_entity;

    if (owner->unk_500.part16.unk_502 & 2)
        entity->state = 6;
    switch (entity->state)
    {
    case 0:
        entity->flipX = 1;
        Sprite_Attach(entity, sBaloreSpriteSheet, (s8)owner->unk_4F8.part8.unk_4FB, owner->unk_510.part8.basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 0x1C;
        entity->unk_500.part8.unk_500 = 0x1B;
        entity->state = 1;
    case 1:
        if (owner->state == 3)
            entity->state = 2;
        break;
    case 2:
        entity->timer = 0x20;
        entity->state = 3;
    case 3:
        if (entity->timer & 2)
            entity->unk_500.part8.unk_500 = 0x1B;
        else
            entity->unk_500.part8.unk_500 = 0x12;
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->unk_500.part8.unk_500 = 0x12;
            entity->timer = 8;
            entity->state = 4;
        }
        break;
    case 4:
        if (entity->timer & 1)
            entity->hidden = 1;
        else
            entity->hidden = 0;
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->hidden = 1;
            entity->state = 5;
        }
        break;
    case 5:
        break;
    case 6:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    entity->palette = owner->palette;
    Anim_StartIndexed(entity, entity->unk_500.part8.unk_500 + sBalorePartAnimOffsets[(s8)owner->unk_4F4.part8.unk_4F6], 2, 0);
    Anim_Update(entity);
}
