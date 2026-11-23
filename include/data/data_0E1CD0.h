#ifndef DATA_0E1CD0_H
#define DATA_0E1CD0_H

/* Data of asm/data/data_0E1CD0.s used by C files. */

#include "types.h"

struct Text12GlyphMetrics {
    u8 sourceBitSkip;
    u8 width;
    u8 pad_2[0x4 - 0x2];
};

struct Text8GlyphMetrics {
    u8 sourceBitSkip;
    u8 width;
    u8 pad_2[0x4 - 0x2];
};

extern const u8 sSpriteSheetDefault1[];
extern const u8 sSpriteSheetDefault2[];
extern const u8 sSpriteSheetDefault4[];
extern const s32 sUnk_080E29A0[3][4][2];
/* First tile of each slot in OBJ_TILES_GRID mode (2D mapping, 32 tiles per
 * row): 4bpp has two columns of 16 tiles x 8 bands of 4 rows, 8bpp one
 * column of 8 bands. */
extern u16 sObjTileGrid4bpp[];
extern u16 sObjTileGrid8bpp[];
extern const void *sAnimStepFuncs[];
extern struct Text12GlyphMetrics sText12GlyphMetrics[];
extern struct Text8GlyphMetrics sText8GlyphMetrics[];
extern const u8 sHitTimers[];
extern const s32 sUnk_080E33FC[3][4][2];

#endif /* DATA_0E1CD0_H */
