/* StolasBeamSegment_Update (Stolas hit box child update). */
#include "code_08009A0.h"
#include "structs/ewram.h"

/**
 * @brief 9C054 | Deleted when the parent (unk_500) is gone or no longer of
 * type 0x61; in state 0 switch the frame at 8 / 16 / 82 frames (0x20 ..
 * 0x22) and delete after 90 frames or when the parent is in state 2.
 */
void StolasBeamSegment_Update(struct EwramData_EntityData *entity)
{
    u8 var_0[0x10];
    struct EwramData_EntityData *stolas = entity->unk_500.unk_500_entity;

    if (stolas != NULL && stolas->updateFunc != NULL && stolas->unk_518.part8.enemyId == 0x61)
    {
        if (entity->state == 0)
        {
            if (entity->timer == 8)
                entity->animFrame = 0x20;
            if (entity->timer == 0x10)
                entity->animFrame = 0x21;
            if (entity->timer == 0x52)
                entity->animFrame = 0x22;
            if (entity->timer++ > 0x59 || stolas->state == 2)
                EntityDelete(entity);
        }
    }
    else
    {
        EntityDelete(entity);
    }
}
