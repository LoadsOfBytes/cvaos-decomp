#include "types.h"
#include "bg_map.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    u8 pad_15;
    s8 scroll;
};

/* Row writers at map column x: the constant part of the address
 * (VRAM + x * 2) is folded into one literal. */
static inline void DrawTileRowAt(s32 x, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    map = (u16 *)((y << 6) + (BG_MAP_VRAM + x * 2));
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}

static inline void FillRowAt(s32 x, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    map = (u16 *)((y << 6) + (BG_MAP_VRAM + x * 2));
    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/* Row writers at a byte offset xo into the map row. */
static inline void DrawTileRowOfs(s32 xo, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(xo + y);
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}

static inline void FillRowOfs(s32 xo, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(xo + y);
    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/* Draw the scroll arrows of an item list: up arrow when scrolled down,
 * down arrow when more than 8 entries remain below.
 * xo = x * 2 sits after the first if/else; GCSE (PRE) copies it into both
 * branches, where it folds to 58, which is the shape of the original. */
void MenuList_DrawScrollArrows(struct MenuPage *page, s32 param_1)
{
    s32 tile = 0xB34D;
    s32 y = 11;
    s32 x;
    s32 xo;

    if (param_1 != 0)
        y = 10;
    x = 29;
    if (page->scroll > 1)
        DrawTileRowAt(x, y, tile, 1);
    else
        FillRowAt(x, y, 0x330D, 1);
    y += 3;
    tile++;
    xo = x * 2;
    if (page->count - 8 > page->scroll)
        DrawTileRowOfs(xo, y, tile, 1);
    else
        FillRowOfs(xo, y, 0x330D, 1);
}
