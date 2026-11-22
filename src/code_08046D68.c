#include "types.h"
#include "syscalls.h"
#include "bg_map.h"
#include "structs/ewram.h"

/**
 * @brief 46D68 | Print value in decimal ending at (x, y) of the BG map,
 * right-aligned, at least digits digits, with the digit tiles 0xF300
 * (0xF100 when gameState.subMode == 9). Returns the number of digits written.
 */
s32 HudMap_PrintRightAlignedNumber(s32 x, s32 y, s32 value, s32 digits)
{
    u16 *map;
    s32 attr;
    s32 i;

    if (gEwramData->gameState.subMode == 9)
        attr = 0xF100;
    else
        attr = 0xF300;
    map = BgMapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        *map = Mod(value, 10) + attr;
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}
