/* 6DD4. */
#include "types.h"

/* Write count consecutive tile numbers starting at tile into the BG map at
 * VRAM 0x0600E000 (32 tiles per row), from (x, y) rightwards. */
void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        tile++;
        map++;
    }
}

/* Clear count consecutive entries of the BG map at VRAM 0x0600E000
 * (32 tiles per row) from (x, y) rightwards. */
void HudMap_ZeroTiles(s32 x, s32 y, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = 0;
        map++;
    }
}
