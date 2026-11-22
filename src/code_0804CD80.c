/* MenuText_Measure: measure a menu text string (Text_GetLocalizedEntry(text) + 2).
 * mode 0: number of characters before the inline character decoder stops
 * (constants 0xFF00/0xF000 kept in locals);
 * otherwise: sum of glyph widths MenuText_GetRangeLength(c - 0x20) over printable
 * characters (c & 0xE0), used by BestiaryMenu_DrawDetailTitle to centre the soul name. */
#include "types.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

/**
 * @brief Measure localized text entry text: mode 0 counts characters until the
 * inline control test stops (original constants kept); otherwise sum the
 * glyph widths (MenuText_GetRangeLength) of the leading characters >= 0x20.
 */
s32 MenuText_Measure(s32 text, s32 mode)
{
    if (mode == 0)
    {
        u8 *p = Text_GetLocalizedEntry(text) + 2;
        s32 i = 0;
        u16 a = 0xFF00, b = 0xF000;
        s32 c, v;
        do
        {
            c = p[i++];
            v = (u8)c & a;
            if (v == b) v = c & 0xFF; else v = -1;
        } while (v == -1);
        return i - 1;
    }
    else
    {
        u8 *p = Text_GetLocalizedEntry(text) + 2;
        s32 i = 0;
        u32 sum = 0;
        s32 c;
        do
        {
            c = p[i++];
            if (c & 0xE0)
            {
                sum += (u16)MenuText_GetRangeLength(c - 0x20);
                c = -1;
            }
        } while (c == -1);
        return sum;
    }
}
