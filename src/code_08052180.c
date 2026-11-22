/* Object14_WallRightCorrection. */
#include "code_08009A0.h"
#include "structs/ewram.h"

static inline s32 Object14_WallRightCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    s32 dx;
    u32 band;
    s32 roomX;
    s32 roomY;
    struct BgLayerState *bg;
    s32 px = x;

    bg = &gEwramData->bgInfo[1];
    roomX = (px - (s16)bg->xPos.part16.integer) << 16;
    roomY = ((y - (s16)bg->yPos.part16.integer) << 16) + 0x40000;
    if ((u32)(entity->posY.whole - roomY) > 0x7FFFF)
        return 0;
    dx = entity->posX.whole - roomX;
    band = dx + 0x180000;
    if (band > 0x300000)
        return 0;
    if (band <= 0x27FFFF)
        return 0;
    entity->unk_4F8.part16.unk_4F8 |= 1;
    {
        s32 v = dx - 0x180000;
        return (v < 0) ? (s16)(-(-v >> 0x10)) : (s16)(v >> 0x10);
    }
}

/**
 * @brief 52180 | Right-hand counterpart of Object14_WallLeftCorrection: within the last
 * 8 px of the 48 px band set bit 0 of unk_4F8 and return the signed offset
 * from the band centre in pixels; otherwise 0.
 */
s16 Object14_WallRightCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return Object14_WallRightCorrectionInline(entity, x, y);
}
