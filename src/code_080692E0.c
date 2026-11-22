/* Update a generic effect with optional damping, random flips and fade. */
#include "code/code_08060B98.h"
#include "particle_effect.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
/**
 * @brief Use unk_4FE as the optional damping divisor, move and apply
 * random flips. Saturate the timer at 250, optionally hide after
 * 40 and optionally animate every other update; delete on animDone.
 */
void Effect_UpdateParticleWithDamping(struct EwramData_EntityData *entity)
{
    if (PARTICLE_FLAGS(entity) & 1)
        EntityDampVelocityX(entity, PARTICLE_DAMPING(entity));
    if (PARTICLE_FLAGS(entity) & 2)
        EntityDampVelocityY(entity, PARTICLE_DAMPING(entity));
    EntityAccelerateAndMove(entity, 0x80000);
    if (PARTICLE_FLIP_FLAGS(entity) & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (PARTICLE_FLIP_FLAGS(entity) & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->timer < 250)
        entity->timer++;
    if (PARTICLE_FLAGS(entity) & 4)
    {
        if (entity->timer <= 40)
            entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
        else
            entity->hidden = 1;
    }
    if (entity->animDone)
        EntityDelete(entity);
    else if (PARTICLE_FLAGS(entity) & 8)
    {
        if (entity->timer & 1)
            Anim_Update(entity);
    }
    else
        Anim_Update(entity);
}

