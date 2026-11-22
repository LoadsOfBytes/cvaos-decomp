/* Object17_CeilingCorrection, Object17_FloorCorrection. */
#include "structs/ewram.h"

/**
 * @brief 53540 | Whether the point (x, y) lies on top of entity (within
 * -0x40..0x40 horizontally of its screen position and up to 0x10 below
 * its top edge y - 0x18, using the integer part of velY): the depth below
 * the edge, else 0.
 */
s32 Object17_CeilingCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    struct BgLayerState *bg;
    s32 sx = x;
    s32 px = (s16)entity->posX.part16.integer;
    s32 dx;
    s32 dy;
    s32 bottom;
    s32 py;

    bg = &gEwramData->bgInfo[1];
    dx = sx - (px + (s16)bg->xPos.part16.integer) + 0x40;
    py = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer + entity->velY.part16.integer;
    bottom = y + 0x18;
    dy = py - bottom;
    if ((u32)dx > 0x80 || (u32)dy > 0x10)
        return 0;
    return (s16)dy;
}

/**
 * @brief 535A0 | As Object17_CeilingCorrection with the next 16.16 y position (posY +
 * velY + camera) and the edge 0x1F above it; returns minus the offset.
 */
s32 Object17_FloorCorrection(struct EwramData_EntityData *entity, s16 x, s16 y)
{
    struct BgLayerState *bg;
    s32 sx = x;
    s32 px = (s16)entity->posX.part16.integer;
    s32 dx;
    s32 dy;
    s32 v;
    s32 sy;

    bg = &gEwramData->bgInfo[1];
    dx = sx - (px + (s16)bg->xPos.part16.integer) + 0x40;
    sy = y;
    v = entity->posY.whole + entity->velY.whole + bg->yPos.whole;
    if (v < 0)
        dy = sy + (-v >> 16);
    else
        dy = sy - (v >> 16);
    dy -= 0x1F;
    if ((u32)dx > 0x80 || (u32)dy > 0x10)
        return 0;
    return (s16)-dy;
}
