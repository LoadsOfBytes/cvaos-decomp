/* Find the ground in eight-pixel steps, then apply its exact correction. */
#include "code_08001194.h"
static inline s16 GroundCorrection(struct EwramData_EntityData *entity, struct BgLayerState *background)
{
    return Collision_GetPushUp((s16)background->xPos.part16.integer + (s16)entity->posX.part16.integer,
                       (s16)background->yPos.part16.integer + entity->posY.part16.integer);
}
void Entity_SnapToGround(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (GroundCorrection(entity, background) < 0)
    {
        while (GroundCorrection(entity, background) != 0)
            entity->posY.part16.integer -= 8;
        entity->posY.part16.integer += 7;
    }
    else
    {
        while (GroundCorrection(entity, background) == 0)
            entity->posY.part16.integer += 8;
    }
    entity->posY.whole += GroundCorrection(entity, background) << 16;
}
