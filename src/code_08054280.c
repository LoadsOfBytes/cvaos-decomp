/* (Matched: the last DMA base is pinned to r0 and added as "b_ - -0x14" so combine keeps the add.)
 * Tile rows and text for the menu at 0x08054280.
 * The first row uses its own counter; later rows reuse the pointer and counter
 * to preserve agbcc register allocation. The final copies retain size 0x18. */
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 sSavePromptBgPalette[];


/**
 * @brief Print the save prompt texts 0x34A/0x34B/0x34D on BG0 and draw the
 * prompt frame tiles (row 5).
 */
void SavePoint_ShowSavePrompt(void)
{
    s32 y = 5;
    u32 base = 0x0600E000;
    s32 i;
    s16 *buf;

    BgPalette_LoadRange(sSavePromptBgPalette, 0, 2, 0xD);
    Text_SetBackgroundColor(0);
    Text12_SetRegion(0, 0, 0x29, 2);
    Text12_ClearRegion();
    Text12_SetCursor(0, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34A), 0);
    Text12_SetCursor(0x10, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
    Text12_SetCursor(0x18, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xAC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E010 + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xCC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E010 + ((y + 4) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xB2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E028 + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xD2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(0x0600E028 + ((y + 4) << 6)));
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
                           (u32 *)(((y + 1) << 6) + ({ register u32 b_ asm("r0") = base; b_ - -0x14; }))); /* FAKEMATCH: pinned register */
    }
}
