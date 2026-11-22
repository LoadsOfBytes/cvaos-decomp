#include "code_08009A0.h"
extern u32 sTextEntries[];

/**
 * @brief Return the indexed language-table entry with offsets 0, 0x3C4 or 0x788 according to language; retain the original unchecked indexing.
 */
u32 Text_GetLocalizedEntry(s32 index)
{
    s32 offset = 0;
    switch (gEwramData->gameState.language)
    {
    case 1: break;
    case 2: offset = 0x3C4; break;
    case 3: offset = 0x788; break;
    }
    return sTextEntries[index + offset];
}
