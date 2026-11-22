/* Resolve directional collisions, apply landing velocity and classify contacts. */
#include "code/code_08060B98.h"
#include "code_08001194.h"
extern s32 Tile_Classify(s16, s16);
static inline s32 PixelCoordinate(s32 origin, s32 value)
{
    if (value < 0) return origin - ((-value) >> 16);
    return origin + (value >> 16);
}
static inline s32 QueryCoordinate(s32 origin, s32 value)
{
    if (value < 0) return (s16)(origin - ((-value) >> 16));
    return (s16)(origin + (value >> 16));
}
/**
 * @brief Accelerate and resolve point collisions with speed limit
 * 0x80000, optionally checking the supplied ceiling offset. Return
 * wall/floor/ceiling flags and tile-class-3 flags 0x40/0x80.
 */
s32 Entity_MoveWithOptionalCeilingCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 checkCeiling)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    EntityAccelerateAndMoveX(entity, 0x80000);
    if (entity->velX.whole < 0)
    {
        correction = Collision_GetPushRight((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 2;
            entity->posX.whole += correction << 16;
        }
    }
    else if (entity->velX.whole > 0)
    {
        correction = Collision_GetPushLeft((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags = 1;
            entity->posX.whole += correction << 16;
        }
    }
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (entity->velY.whole <= 0)
    {
        if (checkCeiling)
        {
            correction = Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
            if (correction)
            {
                flags |= 8;
                entity->posY.whole += correction << 16;
            }
        }
    }
    if (entity->velY.whole >= 0)
    {
        correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
            (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags |= 4;
            entity->posY.whole += correction << 16;
            entity->velY.whole = landingVelocity;
        }
    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}


