/* EventScene31Debris_Update. */
#include "structs/ewram.h"

extern s32 Collision_GetGroundOffset(s32 x, s32 y);

/**
 * @brief 5CB08 | Update of a debris piece: fall with gravity accelY until
 * it touches the terrain, then set deleteRequested.
 */
s32 EventScene31Debris_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg;
    s32 x = (s16)entity->posX.part16.integer;

    bg = &gEwramData->bgInfo[1];
    if (Collision_GetGroundOffset(x + (s16)bg->xPos.part16.integer,
                     (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer))
    {
        entity->deleteRequested = 1;
    }
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
    }
}
