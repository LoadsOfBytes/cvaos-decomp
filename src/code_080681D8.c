/* Find or load a palette in the four shared object slots. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
s8 ObjPalette_FindOrLoad(void *palette)
{
    s8 index;
    for (index = 0; index < 4; index++)
    {
        struct EwramData *ram = gEwramData;
        if ((void *)ram->gameState.sharedObjPaletteSources[index] == palette && ram->gameState.sharedObjPaletteVariants[index] == 0)
            return index + 10;
        if (gEwramData->gameState.sharedObjPaletteSources[index] == 0)
        {
            gEwramData->gameState.sharedObjPaletteSources[index] = (s32)palette;
            gEwramData->gameState.sharedObjPaletteVariants[index] = 0;
            ObjPalette_LoadRange(palette, 0, 1, index + 10);
            return index + 10;
        }
    }
    return -1;
}
