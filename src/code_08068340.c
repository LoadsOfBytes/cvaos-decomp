/* Release the cached slot for a palette and variant. */
#include "structs/ewram.h"
static inline void ReleasePalette(u8 slot)
{
    s8 index = slot - 10;
    if (index >= 0)
    {
        if (index <= 3)
        {
            gEwramData->gameState.sharedObjPaletteSources[index] = 0;
            gEwramData->gameState.sharedObjPaletteVariants[index] = 0;
        }
    }
}
/**
 * @brief Release the shared object palette row that holds (palette, variant).
 */
void SharedObjPalette_Release(void *palette, u8 variant)
{
    s32 index;
    for (index = 0; index < 4; index++)
    {
        if ((void *)gEwramData->gameState.sharedObjPaletteSources[index] == palette && gEwramData->gameState.sharedObjPaletteVariants[index] == variant)
        {
            ReleasePalette(index + 10);
            break;
        }
    }
}
