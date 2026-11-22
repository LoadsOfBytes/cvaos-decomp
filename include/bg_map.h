#ifndef BG_MAP_H
#define BG_MAP_H

#include "types.h"

/* Menu BG screen map in VRAM: 32 u16 entries per row. */
#define BG_MAP_VRAM 0x0600E000

/* Address of map entry (x, y), computed in the original's order (inlined
 * by the number printers HudMap_PrintNumberAttr..HudMap_PrintNumber). */
static inline u16 *BgMapAddr(s32 x, s32 y)
{
    x <<= 1;
    y <<= 6;
    y += BG_MAP_VRAM;
    return (u16 *)(x + y);
}

/* Write count consecutive tile numbers starting at tile from (x, y)
 * rightwards. The original code has this loop inlined in several menu
 * helpers; HudMap_WriteTiles is the same loop
 * as a function. */
static inline void DrawTileRow(s32 x, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}

#endif // BG_MAP_H
