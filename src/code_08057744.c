/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object2AUpdate = Object29Update */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Object2A_UpdateMovement(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 57744 | Same as Object29Update.
 */
void Object2AUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object2A_UpdateMovement(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(collision, entity);
}
