/* Follow the owner's Y after falling onto it; remove if the owner is lost. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "structs/ewram.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
static inline void CopyBit(struct EwramData_EntityData *a, u32 b) { a->hidden = b; }
void Object22FallingPart_Update(struct EwramData_EntityData *entity)
{
    u32 buffer[4];
    struct EwramData_EntityData *owner = entity->ext.object22Part.owner;
    if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
        goto remove;
    if (entity->state == 0)
    {
        CopyBit(entity, owner->hidden);
        entity->posY.whole += entity->velY.whole;
        if (entity->posY.whole >= owner->posY.whole)
        {
            entity->posY.whole = owner->posY.whole;
            entity->state++;
        }
        Hit_CheckWithPlayer(buffer, entity);
    }
    else if (entity->state == 1)
    {
        CopyBit(entity, owner->hidden);
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
    }
    else
    {
    remove:
        EntityDelete(entity);
    }
}
