#ifndef GRAPHICS_INLINE_H
#define GRAPHICS_INLINE_H

#include "types.h"
#include "structs/ewram.h"

/* Inline helper shared by src/engine/obj_gfx.c, src/engine/obj_gfx.c and
 * src/engine/obj_gfx.c (upstream defined it once in engine/obj_gfx.c). */
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
