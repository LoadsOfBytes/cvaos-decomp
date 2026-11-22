/* Movement probe: restore the position unless the movement result has mask 4 set. */
#include "code_08009A0.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *, s32, s32, s32);

static inline s32 ProbeMovement(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit)
{
    return Entity_MoveWithCollisionAlt(entity, landingVelocity, ceilingOffset, speedLimit);
}

/**
 * @brief Optionally face the target, set the walk velocity and probe the
 * movement; restore the position unless the probe allows moving.
 * Return the probe result.
 */
s32 Entity_TryWalk(struct EwramData_EntityData *entity, s32 speed, s32 ceilingOffset, s32 velocityY, s32 skipSetup)
{
    s32 savedX = entity->posX.whole;
    s32 savedY = entity->posY.whole;
    s32 result;
    if (skipSetup == 0 && entity->animTimer == 0) Entity_FaceTarget(entity);
    entity->velY.whole = velocityY;
    if (entity->flipX) entity->velX.whole = speed;
    else entity->velX.whole = -speed;
    result = ProbeMovement(entity, 0x10000, ceilingOffset, 0x80000);
    if (!(result & 4))
    {
        entity->posX.whole = savedX;
        entity->posY.whole = savedY;
    }
    return result;
}
