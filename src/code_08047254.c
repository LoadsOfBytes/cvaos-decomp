#include "types.h"

/* Fill a 16-by-16 block at the top left of the 32-entry-wide BG map.
 * Explicit registers retain agbcc's separate row and tile counters. The
 * inner countdown reuses index; next carries the following outer index. */
void HudMap_DrawTileBlock16(s32 base)
{
    s32 y = 0;
    s32 index = 0;
    s32 x = 0;
    s32 vram = 0x600E000;
    do
    {
        u16 *map = (u16 *)(x + (y * 64 + vram));
        s32 offset = index * 16;
        s32 tile;
        s32 next;
        y++;
        next = index + 1;
        /* Keep offset separate: agbcc otherwise reverses the adds operands. */
        tile = base + offset;
        index = 15;
        do
        {
            *map = tile;
            map++;
            tile++;
            index--;
        } while (index >= 0);
        index = next;
    } while (index <= 15);
}
