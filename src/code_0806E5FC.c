#include "structs/ewram.h"
extern s32 Tile_Classify(s32, s32);

static inline s32 Add16(s16 a, u16 b) { return a + b; }

/* Probe downwards in BG1 space, returning the first offset with tile type 3. */
/**
 * @brief Probe up to 8 px downwards from a screen point and return the offset
 * of the first water surface tile (class 3), else 8. Used by water
 * enemies and the Killer Fish soul.
 */
s32 Collision_FindWaterSurfaceOffset(s16 x_, s16 y_)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u16 offset = 0;
    s32 i = 0;
    s16 x = x_, y = y_;
    do
    {
        s32 px = (s16)Add16(x, bg->xPos.part16.integer);
        s32 py = Add16(y, bg->yPos.part16.integer);
        s32 n = (s16)offset;
        switch (Tile_Classify(px, (s16)(n + py)))
        {
        case 2:
            break;
        case 3:
            return n;
        }
        offset = n + 1;
    } while (i++ <= 7);
    return 8;
}

