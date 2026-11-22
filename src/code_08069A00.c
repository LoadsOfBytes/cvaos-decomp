/* Move an entity and resolve walls, ceiling and floor in background 1. */
#include "code/code_08060B98.h"
#include "code_08001194.h"
static inline s32 PixelCoordinate(s32 origin, s32 value)
{
    if (value < 0) return origin - ((-value) >> 16);
    return origin + (value >> 16);
}
s32 Entity_MoveWithCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit, s16 wallOffset)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    if (entity->velX.whole || entity->accelX.whole)
    {
        EntityAccelerateAndMoveX(entity, speedLimit);
        if (entity->velX.whole < 0)
        {
            correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer - wallOffset,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer - 10);
            if (correction)
            {
                entity->posX.whole += correction << 16;
                flags = 2;
            }
        }
        if (entity->velX.whole > 0)
        {
            correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer + wallOffset,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer - 10);
            if (correction)
            {
                entity->posX.whole += correction << 16;
                flags |= 1;
            }
        }
    }
    if (entity->velY.whole || entity->accelY.whole)
    {
        EntityAccelerateAndMoveY(entity, speedLimit);
        if (entity->velY.whole < 0)
        {
            correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
            if (correction)
            {
                entity->posY.whole += correction << 16;
                flags |= 8;
            }
        }
        if (entity->velY.whole > 0)
        {
            correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
            if (correction)
            {
                entity->posY.whole += correction << 16;
                entity->posY.part16.fraction = 0;
                entity->velY.whole = landingVelocity;
                flags |= 4;
            }
        }
    }
    return flags;
}


