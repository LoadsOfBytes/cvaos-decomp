/* Refresh the entity's collision boxes for the current sprite frame. */
#include "code/code_08040A38.h"
void Entity_UpdateBoxes(struct EwramData_EntityData *entity)
{
    u8 *bank = &entity->spriteSlot;
    struct SpriteSheet *graphics = gEwramData->spriteSlots[*bank].sheet;
    u8 frameIndex = entity->animFrame;
    struct SpriteFrame *frames = graphics->frames;
    struct SpriteFrame *frame = (struct SpriteFrame *)((u32)frameIndex * sizeof(*frame) + (u32)frames);
    void *boxes = frame->boxes;
    switch (frame->boxCount)
    {
    case 1:
        entity->hit.flags &= ~4;
        entity->hit.flags &= ~2;
        HitBox_SetBody(entity, boxes);
        HitBox_SetAttack(entity, boxes);
        break;
    case 2:
        entity->hit.flags &= ~4;
        entity->hit.flags &= ~2;
        HitBox_SetBody(entity, boxes);
        HitBox_SetAttack(entity, (u8 *)boxes + 4);
        break;
    case 0:
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        break;
    }
}


