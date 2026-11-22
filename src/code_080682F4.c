/* Restore the four cached palette rows reserved for shared objects. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
/**
 * @brief Reload the four shared object palette rows 10..13 from their cached
 * sources (after the pause menu).
 */
void SharedObjPalette_RestoreAll(void)
{
    s8 index;
    for(index=0;index<4;index++)
    {
        struct EwramData *ram=gEwramData;
        void *palette=(void *)ram->gameState.sharedObjPaletteSources[index];
        if(palette)ObjPalette_LoadRange(palette,ram->gameState.sharedObjPaletteVariants[index],1,index+10);
    }
}
