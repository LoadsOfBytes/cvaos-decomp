/* Tile rows and text for the menu at 0x080540A8.
 * The first row uses its own counter; later rows reuse the pointer and counter
 * to preserve agbcc register allocation. The final copies retain size 0x18.
 */
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 sSavePromptBgPalette[];


/**
 * @brief Unreferenced variant of SavePoint_ShowSavePrompt drawing at row 0x15
 * with a highlighted second text.
 */
void SavePoint_ShowSavePromptLow(void)
{
    s32 y = 0x15;
    s32 i;
    s16 *buf;

    BgPalette_LoadRange(sSavePromptBgPalette, 0, 2, 0xD);
    Text_SetBackgroundColor(3);
    Text12_SetRegion(0, 0, 0x29, 2);
    Text12_ClearRegion();
    Text12_SetCursor(0, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34A), 0);
    Text_SetBackgroundColor(0xD);
    Text12_SetCursor(0x10, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
    Text_SetBackgroundColor(3);
    Text12_SetCursor(0x18, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
    Text_SetBackgroundColor(3);
    {
        s16 *first = (s16 *)gEwramData->sharedScratchBuffer;
        s32 j;
        for (j = 0; j <= 4; j++)
            *first++ = (j + 0xAC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E00E + ((y + 2) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xCC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E00E + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xB2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E028 + ((y + 2) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xD2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E028 + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 23; i++)
            *buf++ = (i + 0xA0) | (s16)0xE200;
        DmaQueue_DirectCopy(24, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E014 + (y << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 23; i++)
            *buf++ = (i + 0xC0) | (s16)0xE200;
        DmaQueue_DirectCopy(24, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E014 + ((y + 1) << 6)));
    }
}
