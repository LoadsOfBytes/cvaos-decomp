/* Update a generic effect with optional damping, random flips and fade. */
#include "code/code_08060B98.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
/**
 * @brief Generic particle update; option bits in unk_4FC: bit 0/1 damp the
 * x/y velocity, then fall (max 0x80000); unk_4FD bit 0/1 randomize
 * flipX/flipY each frame; unk_4FC bit 2 blinks out over 40 frames.
 * Deleted when the animation ends, otherwise animated.
 */
void Effect_UpdateParticle(struct EwramData_EntityData *entity)
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
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

