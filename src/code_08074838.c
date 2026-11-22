/* Slow and animate a particle, removing it after its timer passes 20. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

/**
 * @brief Debris update: slow down, fall and delete after 20 frames.
 */
void UneDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 8);
    EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    Anim_Update(entity);
    if (entity->timer++ > 0x13)
        EntityDelete(entity);
}
