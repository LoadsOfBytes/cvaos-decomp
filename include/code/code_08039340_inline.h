#ifndef CODE_08039340_INLINE_H
#define CODE_08039340_INLINE_H

#include "types.h"
#include "structs/ewram.h"

/* Inline helper shared by src/code_08039340.c, src/code_08039DC0.c and
 * src/code_0803B66C.c (upstream defined it once in code_08039340.c). */
static inline s32 ObjGfx_FindEntry(s32 tileSlot)
{
    s32 i;

    for (i = gEwramData->objGfxUsedHead; i != -1; i = gEwramData->objGfx[i].next)
    {
        if (gEwramData->objGfx[i].tileSlot == tileSlot)
            return i;
    }
    return -1;
}

#endif
