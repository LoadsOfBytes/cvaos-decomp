#include "types.h"
#include "syscalls.h"
#include "bg_map.h"

/* Body of HudMap_PrintNumberAttr, inlined (attr computed by the caller). */
static inline s32 PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 attr, s32 zeroPad)
{
    u16 *map;
    s32 i;

    if (value < 0)
    {
        DrawTileRow(x, y, attr, 1);
        return 1;
    }

    map = BgMapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        s32 d = Mod(value, 10);
        if (i != 1 && value == 0 && zeroPad == 0)
            *map = attr + 13;
        else
            *map = attr + d;
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

/**
 * @brief 46FFC | Like HudMap_PrintNumberAttr with attr = palette << 12 | 0x300
 * (number printer; the shared body is inlined to match the original order).
 */
s32 HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 palette, s32 zeroPad)
{
    return PrintNumber(x, y, value, digits, palette << 12 | 0x300, zeroPad);
}
