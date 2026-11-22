/* Clamp the entity's X position to an inset measured in eight-pixel units. */
#include "structs/ewram.h"
s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity, s32 inset)
{
    struct BgLayerState *initial = &gEwramData->bgInfo[1];
    struct BgMapMetadata *metadata = initial->pBgMetadata;
    struct BgLayerState *background = initial;
    s32 x = ((s16)entity->posX.part16.integer + (s16)background->xPos.part16.integer) / 8;
    s32 width = metadata->widthCells == 1 ? 30 : metadata->widthCells * 32;
    if (x < inset)
    {
        entity->posX.part16.integer = (inset << 3) - background->xPos.part16.integer;
        entity->posX.part16.fraction = 0;
    }
    else if (x > width - inset)
    {
        entity->posX.part16.integer = ((width - inset + 1) << 3) - background->xPos.part16.integer;
        entity->posX.part16.fraction = 0;
    }
    else return 0;
    return 1;
}
