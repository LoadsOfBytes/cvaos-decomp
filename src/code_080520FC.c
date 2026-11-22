/* Object14_WallLeftCorrection. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "structs/ewram.h"

static inline s32 Object14_WallLeftCorrectionInline(struct EwramData_EntityData *entity, s16 x, s16 y)
{
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
    band = entity->posX.whole - roomX + 0x180000;
    if (band > 0x300000)
        return 0;
    if (band > 0x80000)
        return 0;
    OBJECT14_FLAGS(entity) |= 2;
    {
        s32 v = band;
        return (v < 0) ? (s16)(-(-v >> 0x10)) : (s16)(v >> 0x10);
    }
}

/**
 * @brief 520FC | Contact test against a point given in screen pixels: when
 * the entity is within 8 px below it and at most 8 px to the right of the
 * 24 px left band edge, set bit 1 of unk_4F8 and return the band offset in
 * pixels; otherwise 0.
 */
s16 Object14_WallLeftCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    return Object14_WallLeftCorrectionInline(entity, x, y);
}
