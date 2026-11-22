#include "structs/ewram.h"
#include "syscalls.h"

/* Status screen: draw the four current stats (STR, CON, INT, LCK) as
 * right-aligned numbers (clamped to 999; a negative value draws one blank tile). */
void Menu_DrawCurrentStats(s32 x, s32 y)
{
    s16 *stat = gEwramData->playerState.currentStats;
    s32 i;
    for (i = 0; i <= 3; i++, stat++)
    {
        s32 col = x * 2;
        s32 v = *stat;
        s32 value;
        s32 row;
        u16 base;
        if (v > 1000)
            v = 999;
        row = y + i;
        value = v;
        base = 0x3300;
        if (value < 0)
        {
            u16 tile = base;
            u32 addr = (row << 6) + 0x0600E000;
            u16 *dst = (u16 *)(col + addr);
            s32 j;
            for (j = 0; j < 1; j++)
            {
                *dst = tile;
                tile++;
                dst++;
            }
        }
        else
        {
            u32 addr = (row << 6) + 0x0600E000;
            u16 *dst = (u16 *)(col + addr);
            s32 digit = 1;
            do
            {
                s32 m = Mod(value, 10);
                if (digit != 1 && value == 0)
                    *dst = base + 0xD;
                else
                    *dst = base + m;
                value = Div(value, 10);
                dst--;
                digit++;
            } while (digit <= 3 || value != 0);
        }
    }
}
