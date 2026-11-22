/* Update a generic effect with optional damping, random flips and fade. */
#include "code/code_08060B98.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
/**
 * @brief Apply optional axis damping, movement and random flips,
 * then count down to removal. Flag 4 enables visibility blinking
 * and flag 8 enables animation while the timer is nonzero.
 */
void Effect_UpdateTimedParticle(struct EwramData_EntityData *entity)
{
    if (PARTICLE_FLAGS(entity) & 1)
        EntityDampVelocityX(entity, 8);
    if (PARTICLE_FLAGS(entity) & 2)
        EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (PARTICLE_FLIP_FLAGS(entity) & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (PARTICLE_FLIP_FLAGS(entity) & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    entity->timer--;
    if (PARTICLE_FLAGS(entity) & 4)
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if (entity->timer == 0)
        EntityDelete(entity);
    else if (PARTICLE_FLAGS(entity) & 8)
        Anim_Update(entity);
}

