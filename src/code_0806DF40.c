/* Entity_IsOffsetOutsideRoom. */
#include "code_08001194.h"
#include "structs/ewram.h"

/**
 * @brief 6DF40 | 1 when the point (offsetX, offsetY) relative to the
 * entity lies outside the room of background 1, in 8 px tiles (rooms of one
 * screen are 30 x 26 tiles, larger ones 32 tiles per screen).
 */
s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity, u16 offsetX, u16 offsetY)
{
    struct BgMapMetadata *metadata = gEwramData->bgInfo[1].pBgMetadata;
    s32 tile;
    s32 limit;

    tile = (GetEntityRoomXPositionInteger(entity) + (s16)offsetX) / 8;
    limit = metadata->widthCells == 1 ? 0x1E : metadata->widthCells * 32;
    if (tile >= 0 && tile < limit)
    {
        tile = (GetEntityRoomYPositionInteger(entity) + (s16)offsetY) / 8;
        limit = metadata->heightCells == 1 ? 0x1A : metadata->heightCells * 32;
        if (tile >= 0 && tile < limit)
            return 0;
    }
    return 1;
}
