/* BossDefeat_LoadEffectTiles / BossDefeat_RestoreEffectTiles (swap of the common effect tiles). */
#include "gba.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Common effect graphics; the tiles start 4 bytes in. */
extern u8 sCommonObjectGfx[];
/* Two 0x200-byte tile blocks that temporarily replace them. */
extern u8 sDefeatEffectTiles[];

/**
 * @brief If global flag bit 3 is clear, queue two 0x200-byte blocks
 * of temporary defeat tiles over the common effect OBJ graphics
 * and set the bit. Preserve exact VRAM destinations and copy order.
 */
void BossDefeat_LoadEffectTiles(void)
{
    if (!(gEwramData->gameState.stateFlags & 8))
    {
        u8 *src = sDefeatEffectTiles;
        s32 tileOffset = ObjTiles_GetSlotTile((u8)ObjGfx_Find(sCommonObjectGfx)) * 32;

        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10000 + tileOffset));
        src += 0x200;
        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10400 + tileOffset));
        gEwramData->gameState.stateFlags |= 8;
    }
}

/**
 * @brief If global flag bit 3 is set, queue the original common effect
 * tiles back into the two OBJ VRAM blocks and clear the bit.
 */
void BossDefeat_RestoreEffectTiles(void)
{
    if (gEwramData->gameState.stateFlags & 8)
    {
        u8 *src = &sCommonObjectGfx[4];
        s32 tileOffset = ObjTiles_GetSlotTile((u8)ObjGfx_Find(src - 4)) * 32;

        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10000 + tileOffset));
        src += 0x200;
        DmaQueue_IndirectCopy(0x200, (u32 *)src, (u32 *)(VRAM_BASE + 0x10400 + tileOffset));
        gEwramData->gameState.stateFlags &= ~8;
    }
}
