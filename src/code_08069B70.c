/* Resolve collisions and classify the upper and lower contact points. */
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
 * @brief Move and correct walls, optional ceiling and floor, then
 * classify upper/lower points for tile class 3. Preserve vertical
 * velocity; the second argument is unused.
 */
s32 Entity_MoveWithClassifiedPointCollision(struct EwramData_EntityData *entity, s32 unused, s32 ceilingOffset, s32 checkCeiling)
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
    correction = Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
        (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
    {
        flags |= 4;
        entity->posY.whole += correction << 16;
    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
        (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}


