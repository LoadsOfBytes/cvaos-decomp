#ifndef TEXT12_H
#define TEXT12_H

/* Functions of engine/text12.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

/* Localized text entry (sTextEntries). */
struct unk_08506B38 {
    u16 unk_0;
    u8* unk_2;
    u8 pad_6[0x8 - 0x6];
};

extern void Text12_BuildGlyphRows(u16 ch, u32 skip, u32 width);
extern void Text12_BlitGlyphWithinColumn(u16 ch, u32 skip, u32 width);
extern void Text12_BlitGlyphAcrossColumns(u16 ch, u32 skip, u32 width);
extern void Text12_ClearWithinColumn(u32 width);
extern void Text12_ClearAcrossColumns(u32 width);
extern void Text8_BuildGlyphRows(u16 ch, u32 skip, u32 width);
extern void Text8_BlitGlyphAcrossColumns(u16 ch, u32 skip, u32 width);
extern void Text8_ClearWithinColumn(u32 width);
extern void Text8_ClearAcrossColumns(u32 width);
extern void Text_Initialize(void);
extern void Text12_SetCursor(u16 column, u16 row);
extern void Text12_SetPixelCursor(u16 pixelX, u16 row);
extern void Text12_SetRegion(u16 left, u16 top, u16 width, u16 height);
extern void Text12_Newline(void);
extern void Text8_SetCursor(u16 column, u16 row);
extern void Text8_SetPixelCursor(u16 pixelX, u16 row);
extern void Text8_SetRegion(u16 left, u16 top, u16 width, u16 height);
extern void Text8_Newline(void);
extern s32 Text12_PutChar(u16 ch);
extern s32 Text12_PutCharAt(u16 column, u16 row, u16 ch);
extern s32 Text8_PutChar(u16 ch);
extern s32 Text8_PutCharAt(u16 column, u16 row, u16 ch);
extern void Text12_ClearNextCell(void);
extern void Text12_ClearCell(u16 column, u16 row);
extern void Text12_ClearRegion(void);
extern void Text8_ClearNextCell(void);
extern void Text8_ClearCell(u16 column, u16 row);
extern void Text8_ClearRegion(void);
extern void Text_SetTileBuffer(void *buffer);
extern void Text_SetForegroundColor(s32 color);
extern void Text_SetBackgroundColor(s32 color);
extern s32 Text12_Print(void *text, s32 index);
extern s32 Text8_Print(void *text, s32 index);
extern u16 Text12_GetPixelX(void);
extern u16 Text8_GetPixelX(void);
extern u16 Text12_GetColumn(void);
extern u16 Text8_GetColumn(void);
extern u16 Text12_GetCellOffset(void);
extern u16 Text8_GetCellOffset(void);
extern u16 Text12_GetPixelY(void);
extern u16 Text8_GetPixelY(void);
extern u16 Text12_GetRow(void);
extern u16 Text8_GetRow(void);
extern s32 MenuText_GetRangeLength(s32 index);

#endif /* TEXT12_H */
