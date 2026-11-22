/* Move an entity and resolve walls, ceiling and floor in background 1
 * (Entity_MoveAndCollide). The s32 block copies of the later corrections shorten the
 * life of ceilingOffset so global-alloc keeps it in sl and spills checkCeiling. */
#include "code/code_08060B98.h"
#include "code_08001194.h"
extern s32 Tile_Classify(s16, s16);
static inline s32 PixelCoordinate(s32 origin, s32 value)
{
    if (value < 0) return origin - ((-value) >> 16);
    return origin + (value >> 16);
}
static inline s32 NegativeCoordinate(s32 origin,s32 value)
{
    if (value < 0) return origin + ((-value) >> 16);
    return origin - (value >> 16);
}
static inline s32 QueryCoordinate(s32 origin, s32 value)
{
    if (value < 0) return (s16)(origin - ((-value) >> 16));
    return (s16)(origin + (value >> 16));
}
s32 Entity_MoveAndCollide(struct EwramData_EntityData *entity, s32 sideOffset, s32 ceilingOffset, s32 checkCeiling, s32 speedLimit)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s32 flags = 0;
    s16 correction;
    register s32 queryX;
    EntityAccelerateAndMoveX(entity, speedLimit);
    if (entity->velX.whole <= 0)
    {
        correction = Collision_GetPushRight(NegativeCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset),
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags |= 2;
            entity->posX.whole += correction << 16;
        }
        queryX = NegativeCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset);
    }
    else
    {
        correction = Collision_GetPushLeft(PixelCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset),
                                 (s16)background->yPos.part16.integer + entity->posY.part16.integer);
        if (correction)
        {
            flags |= 1;
            entity->posX.whole += correction << 16;
        }
        queryX = PixelCoordinate((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,sideOffset);
    }
    correction = Collision_GetPushDown(queryX, (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (correction)
        entity->posY.whole += correction << 16;
    EntityAccelerateAndMoveY(entity, speedLimit);
    if (checkCeiling)
    {
    { s32 c2 = (s16)Collision_GetPushDown((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             PixelCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset));
    if (c2)
    {
        flags |= 8;
        entity->posY.whole += c2 << 16;
    }}
    }
    if (entity->velY.whole > 0)
    {
    { s32 c2 = (s16)Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                             (s16)background->yPos.part16.integer + entity->posY.part16.integer);
    if (c2)
    {
        flags |= 4;
        entity->posY.whole += c2 << 16;
    }}
    }
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                      QueryCoordinate((s16)background->yPos.part16.integer, entity->posY.whole + ceilingOffset)) == 3)
        flags |= 0x80;
    if (Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                      (s16)(background->yPos.part16.integer + entity->posY.part16.integer)) == 3)
        flags |= 0x40;
    return flags;
}


