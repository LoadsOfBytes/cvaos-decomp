/* BigGolemRock_Update. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayerSharedHistory(void *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner);

/**
 * @brief AD0B8 | State 0: fall (max 0x40000) with a hitbox owned by
 * unk_4F8_entity; delete when off screen.
 */
void BigGolemRock_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];
    struct EwramData_EntityData *golem = entity->unk_4F8.unk_4F8_entity;

    if (entity->state == 0)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        if (EntityIsOutsideScreen(entity, 1))
            EntityDelete(entity);
        else
            Hit_CheckWithPlayerSharedHistory(collision, entity, golem);
    }
}
