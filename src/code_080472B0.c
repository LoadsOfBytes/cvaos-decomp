#include "code/code_08039340.h"
#include "bg_map.h"

/* Write count consecutive tile numbers from map byte offset xOffset on row y.
 * Modifying y in place keeps loop.c from strength-reducing the caller's row
 * address (the original recomputes it every row). */
static inline void FillMapRow(s32 xOffset, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(xOffset + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        tile++;
        map++;
    }
}

/* Fill a width x height (in 6x12 px text cells) block of the menu tilemap with
 * consecutive tiles (32 per row), then reset the text drawing region. */
void Text12_SetupTileRegion(s32 x, s32 y, u16 tile, s32 width, s32 height)
{
    s32 columns = (width * 6) >> 3;
    s32 rows = (height * 12) >> 3;
    s32 row;
    for (row = 0; row < rows; row++)
    {
        u16 t = tile + row * 32;
        FillMapRow(x * 2, y + row, t, columns);
    }
    Text12_SetRegion(0, 0, width, height);
    Text12_ClearRegion();
}
