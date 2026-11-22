/* Check whether the entity lies beyond the background's tile dimensions. */
#include "structs/ewram.h"
/**
 * @brief Return 1 when the entity position (plus the BG 1 scroll) lies outside the
 * BG 1 map of the room (30 x 26 tiles for a one-screen map, else 32 tiles
 * per screen), else 0.
 */
s32 Entity_IsOutsideRoomMap(struct EwramData_EntityData *entity)
{
    struct BgLayerState *initial = &gEwramData->bgInfo[1];
    struct BgMapMetadata *metadata = initial->pBgMetadata;
    struct BgLayerState *background = initial;
    s32 x = ((s16)entity->posX.part16.integer + (s16)background->xPos.part16.integer) / 8;
    s32 width = metadata->widthCells == 1 ? 30 : metadata->widthCells * 32;
    s32 y, height;
    if (x < 0 || x >= width) goto outside;
    y = (entity->posY.part16.integer + (s16)background->yPos.part16.integer) / 8;
    height = metadata->heightCells == 1 ? 26 : metadata->heightCells * 32;
    if (y < 0 || y >= height) goto outside;
    return 0;
outside:
    return 1;
}
