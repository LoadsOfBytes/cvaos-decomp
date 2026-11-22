/* Scan text commands up to a stop marker, dispatching layout changes. */
#include "code/code_08039340.h"
/**
 * @brief Print text from offset up to a stop marker, handling clear and
 * newline commands, with the 12px (mode != 0) or 8px text routines.
 * Used by TextWindow_Update.
 */
s32 Text_PrintWithCommands(u8 *text, s32 initialOffset, s32 mode)
{
    s32 running = 1;
    u8 *commands = text + 2;
    s32 offset = initialOffset;
    void (*reset)(void);
    void (*advance)(void);
    s32 (*scan)(void *, s32);
    if (mode)
    {
        reset = Text12_ClearRegion;
        advance = Text12_Newline;
        scan = Text12_Print;
    }
    else
    {
        reset = Text8_ClearRegion;
        advance = Text8_Newline;
        scan = Text8_Print;
    }
    while (running)
    {
        offset = scan(text, offset);
        switch (commands[offset])
        {
        case 1:
        case 2:
        case 5:
        case 7:
        case 10:
            return offset;
        case 3:
            offset += 2;
            break;
        case 6:
            offset++;
            advance();
            break;
        case 8:
            {
                s32 parameter = commands[offset + 1];
                offset += 2;
                Text_SetForegroundColor(parameter);
            }
            break;
        case 9:
            offset++;
            reset();
            break;
        case -1:
        case 0:
        case 4:
        default:
            offset++;
            break;
        }
    }
    return offset;
}
