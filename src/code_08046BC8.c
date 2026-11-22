#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void MenuText_PrintActionButton(s32 action);

/**
 * @brief Print the localized entry with newline, button and paired-symbol control codes, then restore the starting logical column/row through Text12_SetCursor.
 */
void MenuText_Print(s32 index)
{
    u32 x = gEwramData->text12Column;
    u32 y = gEwramData->text12Row;
    s32 lines = 0;
    s32 cont = 1;
    u8 *text = Text_GetLocalizedEntry(index);
    u8 *sub = NULL;
    s32 pos = 0;
    s32 subPos = 0;
    s32 i;
    s32 c;

    do
    {
        u8 *p;
        if (sub)
        {
            p = sub;
            i = subPos;
        }
        else
        {
            p = text;
            i = pos;
        }
        p += 2;
        c = Text12_PutChar(p[i]);
        if (c == -1)
        {
            i++;
        }
        else
        {
            switch (c & 0xFF)
            {
            case 6:
                i++;
                lines++;
                Text12_SetCursor(x, y + lines);
                break;
            case 0xB:
                MenuText_PrintActionButton(0);
                i++;
                break;
            case 0xC:
                MenuText_PrintActionButton(1);
                i++;
                break;
            case 0xD:
                MenuText_PrintActionButton(2);
                i++;
                break;
            case 0xE:
                MenuText_PrintActionButton(3);
                i++;
                break;
            case 0xF:
                gEwramData->text12ColumnPixelOffset++;
                gEwramData->text12PixelX++;
                Text12_PutChar(0x88);
                i++;
                break;
            case 0x10:
                gEwramData->text12ColumnPixelOffset++;
                gEwramData->text12PixelX++;
                Text12_PutChar(0x89);
                i++;
                break;
            case 0x11:
                Text12_PutChar(0x8C);
                Text12_PutChar(0x8D);
                i += 2;
                break;
            case 0x12:
                Text12_PutChar(0x8A);
                Text12_PutChar(0x8B);
                i += 2;
                break;
            case 0xA:
                if (!sub)
                {
                    cont = 0;
                    break;
                }
                sub = NULL;
                i = pos;
                i++;
                break;
            default:
                i++;
                break;
            }
        }
        if (sub)
            subPos = i;
        else
            pos = i;
    } while (cont);

    Text12_SetCursor(x, y);
}
