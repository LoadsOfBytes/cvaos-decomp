/* Set up a language-dependent background: copy its map, load the given
 * tiles/palette and layer, and select palette 13 for every map entry. */
#include "gba.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
extern void *memcpy(void *dst, const void *src, unsigned long size);
extern u8 sLocalizedWindowGfx[];
extern u8 sLocalizedWindowGfxLanguage2[];
extern u8 sLocalizedWindowGfxLanguage3[];

static inline u8 *GfxData(void *data)
{
    register u8 *src asm("r2") = NULL; /* FAKEMATCH: src in r2 */

    switch (*(u8 *)data)
    {
        case 1:
            ObjGfx_Decompress(data);
            { register u8 *g_ asm("r0") = (u8 *)gEwramData; /* FAKEMATCH: base in r0 */ src = g_ + 0x153F8; }
            break;
        case 0:
            src = (u8 *)data + 4;
            break;
    }
    return src;
}

static inline u8 *GfxDataK(void *data)
{
    u8 *src = NULL;

    switch (*(u8 *)data)
    {
        case 1:
            ObjGfx_Decompress(data);
            { register u8 *g_ asm("r0") = (u8 *)gEwramData; /* FAKEMATCH: base in r0 */ u32 k_ = 0x153F8; asm("" : "+r"(k_)); /* FAKEMATCH: opaque offset */ src = g_ + k_; }
            break;
        case 0:
            src = (u8 *)data + 4;
            break;
    }
    return src;
}

/**
 * @brief Load language-dependent window graphics, supplied tiles/palette and map layer, restore BG2 setup and OR 0xD100 into the window map entries.
 */
void TextWindow_LoadBackground(void *tiles, u8 *palette, struct BgMapMetadata *layer)
{
    struct BgLayerState bg[4];
    u8 *src;
    u16 *map;
    s32 i;
    s32 n = 0x400;

    
    memcpy(bg, gEwramData->bgInfo, sizeof(bg));
    gDisplayRegisters.bgCnt[0] = 0x1C04;
    switch (gEwramData->gameState.language)
    {
        case 2:
            src = GfxData(sLocalizedWindowGfxLanguage2);
            break;
        case 3:
            src = GfxDataK(sLocalizedWindowGfxLanguage3);
            break;
        default:
            src = GfxDataK(sLocalizedWindowGfx);
            break;
    }
    DMA_COPY_16(3, src + n, (void *)0x06005800, 0x400);
    BgPalette_LoadRange((u8 *)0x080E5C20, 4, 1, 0xF);
    Gfx_CopyResource(tiles, (void *)0x06006000, 1);
    BgPalette_LoadRange(palette, 0, 1, 0xD);
    BgMap_SetupPrimary(0, layer, 0, 0);
    BgMap_Setup(2, bg[2].pBgMetadata, bg[2].xPos.part16.integer, bg[2].yPos.part16.integer);
    gDisplayRegisters.bgOfs[0].hOfs = 4;
    map = (u16 *)0x0600E000;
    for (i = 0; i < 0x400; i++)
        *map++ |= 0xD100;
}
