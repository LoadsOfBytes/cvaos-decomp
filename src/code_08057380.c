/* Object29Update. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Object29_UpdateMovement(struct EwramData_EntityData *entity);

/**
 * @brief 57380 | Object 0x29 update: Player_QueueEntity(entity, 1), state 0
 * Object29_UpdateMovement, animate and Hit_CheckWithPlayer.
 */
void Object29Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object29_UpdateMovement(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(collision, entity);
}
