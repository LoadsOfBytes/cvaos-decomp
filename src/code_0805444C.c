/* Unreferenced H-blank table fill between the save prompt functions (matches with an s16 cast). */
#include "structs/ewram.h"

/**
 * @brief Unreferenced: fill the back H-blank buffer: lines 0..0x23 with 6,
 * 0x24..0x2F with 0x7C + offset, 0x30..0x4F - offset with 0, the rest up
 * to 0x4F with 0x6C + offset, then a 0 terminator, and request the start.
 */
void SavePoint_FillPromptHBlankBuffer(s32 offset)
{
    s32 base = 0xAC;
    s16 *dst = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    s32 i;
    s32 gapEnd;

    for (i = 0; i <= 0x23; i++)
        *dst++ = 6;
    for (i = 0; i <= 0xB; i++)
        *dst++ = (s16)(base - 0x30) + offset;
    i = 0x30;
    gapEnd = 0x50 - offset;
    for (; i < gapEnd; i++)
        *dst++ = 0;
    for (; i <= 0x4F; i++)
        *dst++ = (s16)(base - 0x40) + offset;
    *dst = 0;
    gEwramData->hBlankEffect.requestStart = 1;
}

