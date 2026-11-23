/* MenuText_PrintWithInsertedEntry: print a text line with embedded sub-strings (codes 6 = newline, 0x13 = insert, 10 = end); matched. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

void MenuText_PrintWithInsertedEntry(s32 id, s32 arg)
{
    s32 x = gEwramData->text12Column;
    u16 y = gEwramData->text12Row;
    s32 line = 0;
    s32 running = 1;
    u8 *str = Text_GetLocalizedEntry(id);
    u8 *sub = NULL;
    s32 pos = 0;
    s32 subPos = 0;
    u8 *p;
    s32 i;
    s32 c;

    Text12_ClearRegion();
loop:
    {
        if (sub != NULL)
        {
            p = sub;
            i = subPos;
        }
        else
        {
            p = str;
            i = pos;
        }
        { u8 *q = p + 2; c = Text12_PutChar(q[i]); }
        if (c == -1)
            goto next;
        switch (c & 0xFF)
        {
        case 6:
            i++;
            line++;
            {
                u32 t = x << 16;
                u16 ny = y + line;
                Text12_SetCursor(t >> 16, ny);
            }
            break;
        case 0x13:
            sub = Text_GetLocalizedEntry(arg + 0x38B);
            i = 0;
            break;
        case 10:
            if (sub == NULL)
            {
                running = 0;
                break;
            }
            sub = NULL;
            i = pos;
        default:
        next:
            i++;
            break;
        }
        if (sub != NULL)
            subPos = i;
        else
            pos = i;
    }
    if (running)
        goto loop;
    /* the loop doubles the reference count of y so that it gets r8 */
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        Text12_SetCursor(x, y);
    while (0);
}
