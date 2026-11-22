/* Walk with facing/slope adjustment and restore position when collision
 * or the requested floor-support condition rejects movement. */
#include "structs/ewram.h"
#include "structs/main.h"
#include "code_08001194.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *);

extern s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *, void *, s32, s32);
s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags)
{
    u32 oldX = entity->posX.whole;
    u32 oldY = entity->posY.whole;
    s32 supportMask;
    /* Preserve the original high-register assignment across the collision call. */
    register s32 collisionFlags asm("r10") = 0; /* FAKEMATCH: pinned register */
    s32 hit;
    s32 support;
    if ((flags & 1) && *(u16 *)&entity->animStep == 0)
        Entity_FaceTarget(entity);
    if (!(flags & 2) && !entity->flipX)
        entity->velX.whole = -speed;
    else
        entity->velX.whole = speed;
    if ((flags & 8) && (entity->floorFlags & 0xC0))
    {
        s32 slope = (entity->floorFlags & 0xC0) >> 6;
        if ((entity->velX.whole > 0 && (entity->floorFlags & 4))
            || (entity->velX.whole < 0 && !(entity->floorFlags & 4)))
        {
            switch (slope)
            {
                case 1:
                    entity->velX.whole /= 2;
                    break;
                case 2:
                    entity->velX.whole = entity->velX.whole / 3 * 2;
                    break;
            }
        }
    }
    supportMask = 0x10;
    support = flags & supportMask;
    if (support)
        collisionFlags |= 1;
    if (flags & 0x20)
        collisionFlags |= 2;
    hit = Entity_MoveWithCollisionProfile(entity, profile, 4, collisionFlags);
    if (hit & 3)
    {
        entity->posX.whole = oldX;
        if (hit & 8)
            entity->posY.whole = oldY;
    }
    else if (support && (hit & supportMask))
    {
        entity->posX.whole = oldX;
        entity->posY.whole = oldY;
    }
    if ((flags & 4) && !(hit & 8))
    {
        entity->posX.whole = oldX;
        entity->posY.whole = oldY;
    }
    return hit;
}
