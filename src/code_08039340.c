/* Upstream functions 0x08039340..0x0803981F from the previously unlinked
 * src/code/code_08039340.c, now linked as matching C. */
#include "structs/render_slots.h"
#include "code/code_08039340.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08001194.h"
#include "code_08008750.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "code/code_080E032C.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"

#include "constants/main.h"

#include "data/data_0E0334.h"
#include "data/data_0E3464.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "code/code_08039340_inline.h"

/**
 * @brief 39340 | Reset the graphics cache
 *
 * All 16 entries go to the free list (tileSlot -1); the frame-set list
 * objGfxSetFrames points at the six objGfxFrameBuffers buffers.
 */
void ObjGfx_Init(void)
{
    s32 i;

    DMA_FILL_32(3, 0, &gEwramData->objGfx, 0x140);
    DMA_FILL_32(3, 0, &gEwramData->objGfxSaved, 0x16C);

    for (i = 0; i < 0x10; i++)
    {
        gEwramData->objGfx[i].prev = i - 1;
        gEwramData->objGfx[i].next = i + 1;
        gEwramData->objGfx[i].tileSlot = -1;
    }

    gEwramData->objGfx[0].prev = -1;
    gEwramData->objGfx[0xF].prev = -1;

    gEwramData->objGfxSet = 0;
    gEwramData->objGfxSetList = 2;

    for (i = 0; i < 6; i++)
    {
        gEwramData->objGfxSetFrames[i] = &gEwramData->objGfxFrameBuffers[i]; // TODO: storing address?
    }
}

/**
 * @brief 3943C | Move the entry that owns a tile slot back to the free list
 *
 * Only the cache entry is cleared; the tile slots are freed by the caller
 * (see ObjGfx_Unload).
 *
 * @param tileSlot First tile slot of the entry (0-15)
 */
void ObjGfx_Release(s16 tileSlot)
{
    s32 entry;
    s16 freeHead;
    s16 prev;
    s16 next;
    s32 savedPrev;
    s32 savedNext;

    if (tileSlot > 0xFU)
    {
        return;
    }

    entry = gEwramData->objGfxUsedHead;
    if (entry != -1)
    {
        while (gEwramData->objGfx[entry].tileSlot != tileSlot)
        {
            do {} while(0); /* FAKEMATCH: empty do/while(0) only adds a loop note for the register allocator */
            entry = gEwramData->objGfx[entry].next;
            if (entry == -1)
            {
                return;
            }
        }
    }

    freeHead = gEwramData->objGfxFreeHead;
    prev = gEwramData->objGfx[entry].prev;
    next = gEwramData->objGfx[entry].next;
    if (entry == gEwramData->objGfxUsedHead)
    {
        gEwramData->objGfxUsedHead = next;
        if (next != -1)
        {
            gEwramData->objGfx[next].prev = -1;
        }
    }
    else
    {
        gEwramData->objGfx[prev].next = next;
        if (next != -1)
        {
            gEwramData->objGfx[next].prev = prev;
        }
    }
    if (freeHead != -1)
    {
        gEwramData->objGfx[freeHead].prev = entry;
    }
    gEwramData->objGfx[entry].next = freeHead;
    gEwramData->objGfx[entry].prev = -1;
    gEwramData->objGfxFreeHead = entry;
    if (gEwramData->objGfx[entry].kind == 2)
    {
        gEwramData->objGfxSet = 0;
    }

    savedPrev = gEwramData->objGfx[entry].prev;
    savedNext = gEwramData->objGfx[entry].next;
    DMA_FILL_16(3, 0, &gEwramData->objGfx[entry], 0x14);
    gEwramData->objGfx[entry].prev = savedPrev;
    gEwramData->objGfx[entry].next = savedNext;
    gEwramData->objGfx[entry].tileSlot = -1;
}


/**
 * @brief 39614 | Take a sprite slot from the free list
 *
 * @param sheet Sprite sheet (animation data)
 * @param tileSlot First tile slot of the loaded graphics
 * @return u8 Sprite slot, 0xFF if none is free
 */
u8 Sprite_AllocSlot(void *sheet, s32 tileSlot)
{
    s32 gfxEntry;
    s16 usedHead;
    s16 slot;

    gfxEntry = ObjGfx_FindEntry(tileSlot);

    if (gEwramData->spriteFreeHead != -1)
    {
        slot = gEwramData->spriteFreeHead;
        usedHead = gEwramData->spriteUsedHead;
        gEwramData->spriteFreeHead = gEwramData->spriteSlots[slot].next;
        if (gEwramData->spriteSlots[slot].next != -1)
        {
            gEwramData->spriteSlots[gEwramData->spriteSlots[slot].next].prev = -1;
        }
        if (usedHead != -1)
        {
            gEwramData->spriteSlots[usedHead].prev = slot;
        }
        gEwramData->spriteUsedHead = slot;
        gEwramData->spriteSlots[slot].next = usedHead;
        gEwramData->spriteSlots[slot].prev = -1;
        gEwramData->spriteSlots[slot].gfxEntry = gfxEntry;
        gEwramData->spriteSlots[slot].sheet = sheet;
        gEwramData->spriteSlots[slot].tileSlot = tileSlot;
        return slot;
    }
    return -1;
}

/**
 * @brief 39748 | Unpack the frames of a type 3 frame set into objGfxFrameBuffers
 *
 * Frames stored raw are copied, LZ77 frames are decompressed; the set's
 * header bytes 1-3 are copied to objGfxSetListHeader.
 *
 * @param set Frame set (unk_1 = frame count, unk_4 = frame pointers)
 */
void ObjGfx_DecompressSet(struct EwramData_unk19404 *set)
{
    s32 i;
    struct ObjGfxFrameResource *src;
    struct EwramData_unk19404 *dst;
    s32 length;

    gEwramData->objGfxSet = set;

    for (i = 0; i < set->unk_1; i++)
    {
        src = set->unk_4[i];
        dst = &gEwramData->objGfxFrameBuffers[i];
        // temp_r1 = src->unk_0;
        length = src->widthTiles * src->heightTiles * src->bitsPerPixel * 8 + 4;
        switch (src->kind)
        {
            case 0:
                DMA_COPY_16(3, src, dst, length);
                break;

            case 1:
                LZ77UnCompWram((u32 *) src->compressedData, dst);
                dst->unk_0 = 0;
                dst->unk_1 = src->bitsPerPixel;
                dst->unk_2 = src->widthTiles;
                dst->unk_3 = src->heightTiles;
                break;
        }
    }

    gEwramData->objGfxSetListHeader[0] = set->unk_1;
    gEwramData->objGfxSetListHeader[1] = set->unk_2;
    gEwramData->objGfxSetListHeader[2] = set->unk_3;
}
