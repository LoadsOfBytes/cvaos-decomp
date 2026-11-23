/* Update a generic effect with optional damping, random flips and fade. */
#include "code/code_08060B98.h"
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
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, entity->ext.particle.damping);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, entity->ext.particle.damping);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->timer < 250)
        entity->timer++;
    if (entity->ext.particle.flags & 4)
    {
        if (entity->timer <= 40)
            entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
        else
            entity->hidden = 1;
    }
    if (entity->animDone)
        EntityDelete(entity);
    else if (entity->ext.particle.flags & 8)
    {
        if (entity->timer & 1)
            Anim_Update(entity);
    }
    else
        Anim_Update(entity);
}

