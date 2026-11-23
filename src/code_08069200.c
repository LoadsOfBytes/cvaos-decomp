/* Update a generic effect with optional damping, random flips and fade. */
#include "code/code_08060B98.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
/**
 * @brief Apply optional damping, movement, random flips and visibility
 * fade. On animDone free the affine slot before deleting; otherwise
 * advance animation. No active source creator is established here.
 */
void Effect_UpdateAffineParticle(struct EwramData_EntityData *entity)
{
    if (entity->ext.particle.flags & 1)
        EntityDampVelocityX(entity, 8);
    if (entity->ext.particle.flags & 2)
        EntityDampVelocityY(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->ext.particle.flipFlags & 1)
        entity->flipX = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flipFlags & 2)
        entity->flipY = RandomNumberGenerator() % 2;
    if (entity->ext.particle.flags & 4)
    {
        if (entity->timer < 40)
            entity->timer++;
        entity->hidden = Entity_GetVisibilityPattern(40 - entity->timer);
    }
    if (entity->animDone)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
        Anim_Update(entity);
}

