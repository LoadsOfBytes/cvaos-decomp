/* Reset all four shared palette slots. */
#include "structs/ewram.h"
/**
 * @brief Clear the four shared object palette slots.
 */
void SharedObjPalette_ResetSlots(void)
{
    s32 index;
    for (index = 0; index < 4; index++)
    {
        gEwramData->gameState.sharedObjPaletteSources[index] = 0;
        gEwramData->gameState.sharedObjPaletteVariants[index] = 0;
    }
}
