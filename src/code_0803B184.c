/* ObjGfx_RestoreState: matched (row-copy inlines as in code_08039DC0.c,
   return 0 at the end, i = 0 before slot, separate list variables for case 1/2). */
/* Reload all sprite graphics slots into VRAM after the slot table was saved. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "gba.h"
#include "macros.h"

static inline void *TileAddress(s32 offset)
{
    return VRAM_BASE + 0x10000 + offset;
}

static inline s32 IsColor256(void)
{
    return (gEwramData->objTileFlags & OBJ_TILES_COLOR256) != 0;
}

static inline void CopyRows(s32 tile, u8 *src, s32 width, s32 queued)
{
    u16 dst;
    s32 n;
    u32 size;
    s32 k;

    dst = ObjTiles_GetSlotTile(tile) << 5;
    n = width * 4;
    size = 0x200;
    if (IsColor256() == 1)
    {
        size <<= 1;
    }
    if (queued == 0)
    {
        for (k = 0; k < n; k++)
        {
            DMA_COPY_16(3, src, TileAddress(dst), size);
            src += size;
            dst += 0x400;
        }
    }
    else
    {
        for (k = 0; k < n; k++)
        {
            DmaQueue_IndirectCopy(size, (u32 *)src, TileAddress(dst));
            src += size;
            dst += 0x400;
        }
    }
}

static inline void LoadTiles(u8 *data, s32 tile, s32 width, u32 queued)
{
    switch (data[0])
    {
        case 1:
            ObjGfx_Decompress(data);
            CopyRows(tile, gEwramData->gfxDecompTiles, width, queued);
            break;

        case 0:
            CopyRows(tile, data + 4, width, queued);
            break;
    }
}

s32 ObjGfx_RestoreState(void)
{
    s32 i;
    s32 j;
    s32 tile;
    s32 width;
    u8 **list;
    u8 **sharedList;
    struct ObjGfxEntry *entry;

    if (gEwramData->objGfxSaved != 0)
    {
    entry = gEwramData->objGfx;
    for (i = 0; i < 16; i++, entry++)
    {
        if (entry->tileSlot != -1 && entry->lifetime == 2)
            ObjTiles_Free(entry->tileSlot);
    }
    DMA_COPY_32(3, (u8 *)gEwramData + 0x11078, (u8 *)gEwramData + 0x11070, 8);
    DMA_COPY_32(3, (u8 *)gEwramData + 0x10008, (u8 *)gEwramData + 0xFEC4, 0x140);
    DMA_COPY_32(3, (u8 *)gEwramData + 0x10154, (u8 *)gEwramData + 0x25468, 0x1C);
    gEwramData->objGfxSet = NULL;
    if (gEwramData->objGfxSetBackup != NULL)
        ObjGfx_DecompressSet(gEwramData->objGfxSetBackup);
    for (i = 0, entry = gEwramData->objGfx; i < 16; i++, entry++)
    {
        if (entry->tileSlot == -1)
            continue;
        if (entry->lifetime != 0)
            continue;
        width = entry->size;
        tile = ObjTiles_AllocAt(entry->tileSlot, width, entry->count);
        switch (entry->kind)
        {
        case 0:
            LoadTiles(entry->source, tile, width, 0);
            break;
        case 1:
            list = (u8 **)((u8 *)entry->source + 4);
            for (j = 0; j < entry->count; j++)
                LoadTiles(list[entry->frames[j]], tile + width * j, width, 0);
            break;
        case 2:
            sharedList = (u8 **)gEwramData->objGfxSetFrames;
            for (j = 0; j < entry->count; j++)
                LoadTiles(sharedList[entry->frames[j]], tile + width * j, width, 0);
            break;
        }
    }
    gEwramData->objGfxSaved = 0;
    return 1;
    }
    return 0;
}
