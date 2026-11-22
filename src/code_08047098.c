/* BG map helpers 0x08047098..0x08047253.
 * All of them write the BG screen map at VRAM 0x0600E000
 * (32 entries per row); 0x330D is the blank tile used by the menus. */
#include "types.h"
#include "bg_map.h"
#include "gba.h"
#include "macros.h"
#include "syscalls.h"
#include "structs/ewram.h"

extern const s32 sHudMapTiles[3];
extern const s32 sHudMapPalettes[3];

/* Fill count entries with the blank tile from (x, y) leftwards. */
void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = 0x330D;
        map--;
    }
}

/* Print value in decimal ending at (x, y), right-aligned, using the
 * two-row digit tiles of the given palette; at least digits digits. */
void HudMap_PrintLargeNumber(s32 x, s32 y, s32 value, s32 digits, s32 palette)
{
    u16 *map;
    s32 attr;
    s32 i;

    attr = (palette << 12) | 0x300;
    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 1; i <= digits || value != 0; i++)
    {
        s32 digit = Mod(value, 10);

        map[0] = attr + 0x10 + digit;
        map[0x20] = attr + 0x20 + digit;
        value = Div(value, 10);
        map--;
    }
}

/* Fill count entries with the blank tile from (x, y) rightwards. */
void HudMap_ClearTiles(s32 x, s32 y, s32 count)
{
    u16 tile = 0x330D;
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/* Fill the whole 32x32 map with the blank tile. */
void HudMap_ClearAll(void)
{
    DMA_FILL_32(3, 0x330D330D, 0x0600E000, 0x800);
}

/* Put the tile selected by index from the two ROM tables at (x, y)
 * (written out: the inlined DrawTileRow allocates registers differently). */
void HudMap_DrawStatChangeIndicator(s32 x, s32 y, s32 index)
{
    s32 tiles[3];
    s32 palettes[3];
    u16 tile;
    s32 attr;
    u16 *map;
    s32 i;

    memcpy(tiles, sHudMapTiles, sizeof(tiles));
    memcpy(palettes, sHudMapPalettes, sizeof(palettes));
    attr = (palettes[index] << 12) | 0x300;
    tile = attr + (tiles[index] + 0x4A);
    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < 1; i++)
    {
        *map = tile;
        tile++;
        map++;
    }
}

/* Draw the 4-tile indicator selected by bits 0..2 of statusEffects at (x, y). */
void HudMap_DrawStatusConditionIcon(s32 x, s32 y)
{
    s32 n = 0;
    s32 flags = gEwramData->playerStatus.statusEffects;

    if (flags & 1)
        n = 2;
    else if (flags & 2)
        n = 3;
    else if (flags & 4)
        n = 1;

    DrawTileRow(x, y, (n << 4) | 0x330A, 4);
}

