/* Move an entity and resolve walls, ceiling and floor in background 1. */
#include "code/code_08060B98.h"
#include "code_08001194.h"
static inline s32 PixelCoordinate(s32 origin, s32 value)
{
    if (value < 0) return origin - ((-value) >> 16);
    return origin + (value >> 16);
}
/**
 * @brief Move at limit 0x80000 and resolve point walls plus both
 * ceiling and floor regardless of vertical direction. At zero
 * X velocity query both walls; floor correction sets landing velocity.
 */
s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    EntityAccelerateAndMoveX(entity, 0x80000);
    if (entity->velX.whole <= 0)
    {
        correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            entity->posX.whole += correction << 16;
            flags = 2;
        }
    }
    if (entity->velX.whole >= 0)
    {
        correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            entity->posX.whole += correction << 16;
            flags |= 1;
        }
    }
    EntityAccelerateAndMoveY(entity, 0x80000);
    correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
    if (correction)
    {
        entity->posY.whole += correction << 16;
        flags |= 8;
    }
    correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
    {
        entity->posY.whole += correction << 16;
        entity->velY.whole = landingVelocity;
        flags |= 4;
    }
    return flags;
}


