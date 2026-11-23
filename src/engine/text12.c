/*
 * 12-pixel text renderer: glyph blitting, clearing and localized entries.
 */
#include "structs/render_slots.h"
#include "gba.h"
#include "text12.h"
#include "data/data_4F1444.h"
#include "data/data_0E1CD0.h"

/* Upstream file split 0x0803FEA4..0x08041433 from the previously unlinked
 * src/engine/obj_gfx.c, now linked as matching C.
 * gTextGlyphRows (IWRAM, named sUnk_03001A18 in the draft) is placed by linker.ld. */

extern s32 gTextGlyphRows[];

/**
 * @brief Convert the selected bitmap rows into twelve packed 4bpp scratch words using source-bit offset, width and foreground/background colors.
 */
void Text12_BuildGlyphRows(u16 ch, u32 skip, u32 width)
{
    u8 *glyph;
    s32 fgColor;
    s32 shift;
    s32 i;
    s32 row;
    u32 j;
    u32 bgFill;
    u8 bits;
    u8 (*font)[0xE];
    s32 glyphIndex;

    fgColor = gEwramData->textForegroundColor;
    font = sText12Font;
    glyphIndex = ch - 0x20;
    glyph = font[glyphIndex];

    bgFill = gEwramData->textBackgroundColor + (gEwramData->textBackgroundColor * 0x10); // gEwramData->textBackgroundColor * 0x11
    bgFill = bgFill & 0xFF;
    bgFill = bgFill | (bgFill << 8);
    bgFill = bgFill | (bgFill << 0x10);

    for (i = 0; i < 0xC; i++)
    {
        row = bgFill;
        bits = glyph[i];

        for (j = 0; j < skip; j++)
        {
            bits *= 2;
        }

        for (j = 0; j < width; j++)
        {
            if (bits & 0x80)
            {
                shift = j * 4;
                row = (row & ~(0xF << shift)) | (fgColor << shift);
            }
            bits *= 2;
        }

        gTextGlyphRows[i] = row;
    }
}

/**
 * @brief Build glyph rows and merge them at the cursor within one tile column, handling the vertical tile-row boundary.
 */
void Text12_BlitGlyphWithinColumn(u16 ch, u32 skip, u32 width)
{
    s32 split;
    s32 tileY;
    s32 mask;
    s32 rowOffset;
    s32 pixelX;
    s32 keepMask;
    s32 i;
    void *tile;
    s32 word;

    pixelX = gEwramData->text12PixelX & 7;
    Text12_BuildGlyphRows(ch, skip, width);

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text12PixelX >> 3) * 0x20) + ((gEwramData->text12PixelY >> 3) * 0x400);

    mask = 0;
    for (i = pixelX; i < pixelX + width; i++)
    {
        mask |= 0xF << (i * 4);
    }
    keepMask = ~mask;

    rowOffset = 0;
    tileY = gEwramData->text12PixelY & 7;
    split = 8 - tileY;
    for (i = 0; i < 12; i++)
    {
        if (i == split)
        {
            rowOffset = 0x400;
            tileY -= 8;
        }

        word = ((s32*)(tile + rowOffset + (i + tileY) * 4))[0];
        word &= keepMask;
        word |= (gTextGlyphRows[i] << (pixelX * 4)) & mask;
        ((s32*)(tile + rowOffset + (i + tileY) * 4))[0] = word;
    }
}

/**
 * @brief Build glyph rows and merge them across two horizontally adjacent tiles, also handling the vertical tile-row boundary.
 */
void Text12_BlitGlyphAcrossColumns(u16 ch, u32 skip, u32 width)
{
    void *tile;
    s32 split;
    s32 keepMask;
    s32 keepRightMask;
    s32 rightMask;
    s32 pixelX;
    s32 mask;
    s32 i;
    s32 tileY;
    s32 rowOffset;
    s32 left;
    s32 right;

    pixelX = gEwramData->text12PixelX & 7;
    Text12_BuildGlyphRows(ch, skip, width);

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text12PixelX >> 3) * 0x20) + ((gEwramData->text12PixelY >> 3) * 0x400);

    mask = 0;
    for (i = pixelX; i < 8; i++)
    {
        mask |= 0xF << (i * 4);
    }
    keepMask = ~mask;

    rightMask = 0;
    for (i = 0; i < ((pixelX + width) - 8); i++)
    {
        rightMask |= 0xF << (i * 4);
    }
    keepRightMask = ~rightMask;

    rowOffset = 0;
    tileY = gEwramData->text12PixelY & 7;
    split = 8 - tileY;
    for (i = 0; i < 12; i++)
    {
        if (i == split)
        {
            rowOffset = 0x400;
            tileY -= 8;
        }

        left = ((s32*)(tile + rowOffset + (i + tileY) * 4))[0];
        right = ((s32*)(tile + rowOffset + (i + tileY) * 4))[8];

        left &= keepMask;
        right &= keepRightMask;

        left |= (gTextGlyphRows[i] << (pixelX * 4)) & mask;
        right |= (gTextGlyphRows[i] >> ((8 - pixelX) * 4)) & rightMask;

        ((s32*)(tile + rowOffset + (i + tileY) * 4))[0] = left;
        ((s32*)(tile + rowOffset + (i + tileY) * 4))[8] = right;
    }
}

/**
 * @brief Clear the supplied pixel width at the cursor within one tile column over twelve rows, preserving pixels outside the mask.
 */
void Text12_ClearWithinColumn(u32 width)
{
    s32 split;
    s32 mask;
    s32 tileY;
    s32 rowOffset;
    s32 pixelX;
    s32 i;
    void *tile;
    s32 word;

    pixelX = gEwramData->text12PixelX & 7;

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text12PixelX >> 3) * 0x20) + ((gEwramData->text12PixelY >> 3) * 0x400);
    
    mask = 0;
    for (i = pixelX; i < pixelX + width; i++)
    {
        mask |= 0xF << (i * 4);
    }
    mask = ~mask;

    rowOffset = 0;
    tileY = gEwramData->text12PixelY & 7;
    split = 8 - tileY;
    for (i = 0; i < 12; i++)
    {
        if (i == split)
        {
            rowOffset = 0x400;
            tileY -= 8;
        }

        word = ((s32*)(tile + rowOffset + (i + tileY) * 4))[0];
        word &= mask;
        ((s32*)(tile + rowOffset + (i + tileY) * 4))[0] = word;
    }
}

/**
 * @brief Clear the supplied pixel width across two tile columns over twelve rows, preserving pixels outside both masks.
 */
void Text12_ClearAcrossColumns(u32 width)
{
    s32 pixelX;
    s32 split;
    s32 mask;
    s32 rightMask;
    s32 tileY;
    s32 i;
    s32 rowOffset;
    void *tile;
    s32 keepMask;
    s32 keepRightMask;
    s32 left;
    s32 right;

    pixelX = gEwramData->text12PixelX & 7;

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text12PixelX >> 3) * 0x20) + ((gEwramData->text12PixelY >> 3) * 0x400);

    mask = 0;
    for (i = pixelX; i < 8; i++)
    {
        mask |= 0xF << (i * 4);
    }
    keepMask = ~mask;

    rightMask = 0;
    for (i = 0; i < ((pixelX + width) - 8); i++)
    {
        rightMask |= 0xF << (i * 4);
    }
    keepRightMask = ~rightMask;

    rowOffset = 0;
    tileY = gEwramData->text12PixelY & 7;
    split = 8 - tileY;
    for (i = 0; i < 12; i++)
    {
        if (i == split)
        {
            rowOffset = 0x400;
            tileY -= 8;
        }

        left = ((s32*)(tile + rowOffset + (i + tileY) * 4))[0];
        right = ((s32*)(tile + rowOffset + (i + tileY) * 4))[8];

        left &= keepMask;
        right &= keepRightMask;

        ((s32*)(tile + rowOffset + (i + tileY) * 4))[0] = left;
        ((s32*)(tile + rowOffset + (i + tileY) * 4))[8] = right;
    }
}

/**
 * @brief Convert the selected bitmap rows into eight packed 4bpp scratch words using source-bit offset, width and foreground/background colors.
 */
void Text8_BuildGlyphRows(u16 ch, u32 skip, u32 width)
{
    u8 *glyph;
    s32 fgColor;
    s32 shift;
    s32 i;
    s32 row;
    u32 j;
    s32 bgFill;
    u8 bits;
    u8 (*font)[0xA];
    s32 glyphIndex;

    fgColor = gEwramData->textForegroundColor;
    font = sText8Font;
    glyphIndex = ch - 0x20;
    glyph = font[glyphIndex];

    bgFill = gEwramData->textBackgroundColor + (gEwramData->textBackgroundColor * 0x10); // gEwramData->textBackgroundColor * 0x11
    bgFill = bgFill & 0xFF;
    bgFill = bgFill | (bgFill << 8);
    bgFill = bgFill | (bgFill << 0x10);

    for (i = 0; i < 8; i++)
    {
        row = bgFill;
        bits = glyph[i];

        for (j = 0; j < skip; j++)
        {
            bits *= 2;
        }

        for (j = 0; j < width; j++)
        {
            if (bits & 0x80)
            {
                shift = j * 4;
                row = (row & ~(0xF << shift)) | (fgColor << shift);
            }
            bits *= 2;
        }
        gTextGlyphRows[i] = row;
    }
}

/**
 * @brief Build eight glyph rows and merge them across two horizontally adjacent tiles at the current cursor.
 */
void Text8_BlitGlyphAcrossColumns(u16 ch, u32 skip, u32 width)
{
    void *tile;
    s32 keepMask;
    s32 mask;
    s32 pixelX;
    s32 i;
    s32 rightMask;
    s32 keepRightMask;
    s32 left;
    s32 right;

    pixelX = gEwramData->text8PixelX & 7;
    Text8_BuildGlyphRows(ch, skip, width);

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text8PixelX >> 3) * 0x20) + (gEwramData->text8Row * 0x400);

    mask = 0;
    for (i = pixelX; i < 8; i++)
    {
        mask |= 0xF << (i * 4);
    }
    keepMask = ~mask;

    rightMask = 0;
    for (i = 0; i < ((pixelX + width) - 8); i++)
    {
        rightMask |= 0xF << (i * 4);
    }
    keepRightMask = ~rightMask;

    for (i = 0; i < 8; i++)
    {
        left = ((u32*)(tile))[i + 0];
        right = ((u32*)(tile))[i + 8];

        left &= keepMask;
        right &= keepRightMask;

        left |= ((gTextGlyphRows[i] << (pixelX * 4)) & mask);
        right |= ((gTextGlyphRows[i] >> ((8 - pixelX) * 4)) & rightMask);

        ((u32*)(tile))[i + 0] = left;
        ((u32*)(tile))[i + 8] = right;
    }
}

/**
 * @brief Clear the supplied pixel width at the cursor within one tile column over eight rows, preserving pixels outside the mask.
 */
void Text8_ClearWithinColumn(u32 width)
{
    s32 mask;
    s32 pixelX;
    s32 i;
    void *tile;
    s32 keepMask;
    s32 word;

    pixelX = gEwramData->text8PixelX & 7;

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text8PixelX >> 3) * 0x20) + (gEwramData->text8Row * 0x400);

    mask = 0;
    for (i = pixelX; i < pixelX + width; i++)
    {
        mask |= 0xF << (i * 4);
    }
    keepMask = ~mask;

    for (i = 0; i < 8; i++)
    {
        word = ((s32*)(tile))[i + 0];
        word &= keepMask;
        ((s32*)(tile))[i + 0] = word;
    }
}

/**
 * @brief Clear the supplied pixel width across two tile columns over eight rows, preserving pixels outside both masks.
 */
void Text8_ClearAcrossColumns(u32 width)
{
    s32 mask;
    s32 i;
    s32 rightMask;
    s32 keepRightMask;
    s32 pixelX;
    s32 keepMask;
    void *tile;
    s32 left;
    s32 right;

    pixelX = gEwramData->text8PixelX & 7;

    tile = (void*)gEwramData->textTileBuffer + ((gEwramData->text8PixelX >> 3) << 5) + (gEwramData->text8Row << 0xA);

    mask = 0;
    for (i = pixelX; i < 8; i++)
    {
        mask |= 0xF << (i * 4);
    }
    keepMask = ~mask;

    rightMask = 0;
    for (i = 0; i < ((pixelX + width) - 8); i++)
    {
        rightMask |= 0xF << (i * 4);
    }
    keepRightMask = ~rightMask;

    for (i = 0; i < 8; i++)
    {
        left = ((s32*)tile)[i + 0];
        right = ((s32*)tile)[i + 8];

        left &= keepMask;
        right &= keepRightMask;

        ((s32*)tile)[i + 0] = left;
        ((s32*)tile)[i + 8] = right;
    }
}

/**
 * @brief Set the tile buffer to VRAM base, reset both cursors and regions, initialize unk_1159C/unk_1159E to 256 and set foreground 1/background 0; the two auxiliary fields remain unresolved.
 */
void Text_Initialize(void)
{
    gEwramData->textTileBuffer = (void *)0x06000000;
    gEwramData->unk_1159C = 0x100;
    gEwramData->unk_1159E = 0x100;
    gEwramData->text12RegionLeft = 0;
    gEwramData->text12Column = 0;
    gEwramData->text12RegionTop = 0;
    gEwramData->text12Row = 0;
    gEwramData->text12LastColumn = 0x28;
    gEwramData->text12LastRow = 0x14;
    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelY = 0;
    gEwramData->text12PixelX = 0;
    gEwramData->text8RegionLeft = 0;
    gEwramData->text8Column = 0;
    gEwramData->text8RegionTop = 0;
    gEwramData->text8Row = 0;
    gEwramData->text8LastColumn = 0x28;
    gEwramData->text8LastRow = 0x1F;
    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelY = 0;
    gEwramData->text8PixelX = 0;
    gEwramData->textForegroundColor = 1;
    gEwramData->textBackgroundColor = 0;
}

static inline u16 Text12_GetColumnInline()
{
    return gEwramData->text12Column;
}

static inline u16 Text12_GetRowInline()
{
    return gEwramData->text12Row;
}

static inline u16 Text8_GetColumnInline()
{
    return gEwramData->text8Column;
}

static inline u16 Text8_GetRowInline()
{
    return gEwramData->text8Row;
}

/**
 * @brief Set clamped column/row, clear the within-cell offset and derive pixel coordinates using 6 by 12 spacing.
 */
void Text12_SetCursor(u16 column, u16 row)
{
    gEwramData->text12Column = column;
    gEwramData->text12Row = row;

    if (gEwramData->text12Column > 0x28)
    {
        gEwramData->text12Column = 0x28;
    }

    if (gEwramData->text12Row > 0x14)
    {
        gEwramData->text12Row = 0x14;
    }

    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
    gEwramData->text12PixelY = Text12_GetRowInline() * 12;
}

/**
 * @brief Set clamped pixel X and row, split X into six-pixel column and remainder, and derive pixel Y using 12-row spacing.
 */
void Text12_SetPixelCursor(u16 pixelX, u16 row)
{
    if (pixelX > 0xF0)
    {
        pixelX = 0xF0;
    }

    if (row > 0x14)
    {
        row = 0x14;
    }

    gEwramData->text12Column = pixelX / 6;
    gEwramData->text12ColumnPixelOffset = pixelX % 6;

    gEwramData->text12PixelX = pixelX;
    gEwramData->text12Row = row;
    gEwramData->text12PixelY = row * 12;
}

/**
 * @brief Store and clamp region bounds using the original width-minus-two and height-minus-one formulas, then recompute current pixel coordinates.
 */
void Text12_SetRegion(u16 left, u16 top, u16 width, u16 height)
{
    gEwramData->text12RegionLeft = left;
    gEwramData->text12RegionTop = top;

    gEwramData->text12LastColumn = (left + width) - 2;
    gEwramData->text12LastRow = (top + height) - 1;

    if (gEwramData->text12RegionLeft > 0x28)
    {
        gEwramData->text12RegionLeft = 0x28;
    }

    if (gEwramData->text12RegionTop > 0x14)
    {
        gEwramData->text12RegionTop = 0x14;
    }

    if (gEwramData->text12LastColumn > 0x28)
    {
        gEwramData->text12LastColumn = 0x28;
    }

    if (gEwramData->text12LastRow > 0x14)
    {
        gEwramData->text12LastRow = 0x14;
    }

    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = gEwramData->text12Column * 6;
    gEwramData->text12PixelY = gEwramData->text12Row * 12;
}

/**
 * @brief Reset X to region start, advance the row with wrapping, clear the within-cell offset and derive pixel coordinates.
 */
void Text12_Newline(void)
{
    gEwramData->text12Column = gEwramData->text12RegionLeft;

    gEwramData->text12Row += 1;
    if (gEwramData->text12Row > gEwramData->text12LastRow)
    {
        gEwramData->text12Row = gEwramData->text12RegionTop;
    }

    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
    gEwramData->text12PixelY = Text12_GetRowInline() * 12;
}

/**
 * @brief Set clamped column/row, clear the within-cell offset and derive pixel coordinates using 6 by 8 spacing.
 */
void Text8_SetCursor(u16 column, u16 row)
{
    gEwramData->text8Column = column;
    gEwramData->text8Row = row;

    if (gEwramData->text8Column > 0x28)
    {
        gEwramData->text8Column = 0x28;
    }

    if (gEwramData->text8Row > 0x1F)
    {
        gEwramData->text8Row = 0x1F;
    }

    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
}

/**
 * @brief Set clamped pixel X and row and split X into column/remainder; preserve the original pixel-Y calculation of row times 12.
 */
void Text8_SetPixelCursor(u16 pixelX, u16 row)
{
    if (pixelX > 0xF0)
    {
        pixelX = 0xF0;
    }

    if (row > 0x1F)
    {
        row = 0x1F;
    }

    gEwramData->text8Column = pixelX / 6;
    gEwramData->text8ColumnPixelOffset = pixelX % 6;

    gEwramData->text8PixelX = pixelX;
    gEwramData->text8Row = row;
    gEwramData->text8PixelY = row * 0xC;
}

/**
 * @brief Store and clamp region bounds using the original width-minus-two and height-minus-one formulas, then recompute current pixel coordinates.
 */
void Text8_SetRegion(u16 left, u16 top, u16 width, u16 height)
{
    gEwramData->text8RegionLeft = left;
    gEwramData->text8RegionTop = top;

    gEwramData->text8LastColumn = (left + width) - 2;
    gEwramData->text8LastRow = (top + height) - 1;

    if (gEwramData->text8RegionLeft > 0x28)
    {
        gEwramData->text8RegionLeft = 0x28;
    }

    if (gEwramData->text8RegionTop > 0x1F)
    {
        gEwramData->text8RegionTop = 0x1F;
    }

    if (gEwramData->text8LastColumn > 0x28)
    {
        gEwramData->text8LastColumn = 0x28;
    }

    if (gEwramData->text8LastRow > 0x1F)
    {
        gEwramData->text8LastRow = 0x1F;
    }

    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = gEwramData->text8Column * 6;
    gEwramData->text8PixelY = gEwramData->text8Row * 8;
}

/**
 * @brief Reset X to region start, advance the row with wrapping, clear the within-cell offset and derive pixel coordinates.
 */
void Text8_Newline(void)
{
    gEwramData->text8Column = gEwramData->text8RegionLeft;
    gEwramData->text8Row += 1;
    if (gEwramData->text8Row > gEwramData->text8LastRow)
    {
        gEwramData->text8Row = gEwramData->text8RegionTop;
    }
    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
}

static inline void Text12_SyncPixelCursor()
{
    gEwramData->text12PixelY = Text12_GetRowInline() * 0xC;
    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
}

/**
 * @brief Return the value unchanged when bits 5..7 are clear; otherwise render the proportional glyph, advance and wrap the cursor, and return -1.
 */
s32 Text12_PutChar(u16 ch)
{
    s32 skip;
    s32 width;
    s32 glyphIndex;

    if (!(ch & 0xE0))
    {
        return ch;
    }

    glyphIndex = ch - 0x20;
    skip = sText12GlyphMetrics[glyphIndex].sourceBitSkip;
    width = sText12GlyphMetrics[glyphIndex].width;
    if (((gEwramData->text12PixelX & 7) + width) <= 7)
    {
        Text12_BlitGlyphWithinColumn(ch, skip, width);
    }
    else
    {
        Text12_BlitGlyphAcrossColumns(ch, skip, width);
    }

    gEwramData->text12ColumnPixelOffset += width;
    if (gEwramData->text12ColumnPixelOffset > 5)
    {
        gEwramData->text12ColumnPixelOffset -= 6;

        gEwramData->text12Column += 1;
        if (gEwramData->text12Column > gEwramData->text12LastColumn)
        {
            gEwramData->text12Column = gEwramData->text12RegionLeft;

            gEwramData->text12Row += 1;
            if (gEwramData->text12Row > gEwramData->text12LastRow)
            {
                gEwramData->text12Row = gEwramData->text12RegionTop;
            }

            // gEwramData->text12PixelY = gEwramData->text12Row * 0xC;
            // gEwramData->text12ColumnPixelOffset = 0;
            // gEwramData->text12PixelX = gEwramData->text12Column * 6;
            Text12_SyncPixelCursor();
        }
        else
        {
            gEwramData->text12PixelX += width;
        }
    }
    else
    {
        gEwramData->text12PixelX += width;
    }

    return -1;
}

/**
 * @brief Set cursor column/row clamped to region upper bounds, clear the within-cell offset and render one value through Text12_PutChar.
 */
s32 Text12_PutCharAt(u16 column, u16 row, u16 ch)
{
    if (column > gEwramData->text12LastColumn)
    {
        column = gEwramData->text12LastColumn;
    }

    if (row > gEwramData->text12LastRow)
    {
        row = gEwramData->text12LastRow;
    }

    gEwramData->text12Column = column;
    gEwramData->text12Row = row;

    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
    gEwramData->text12PixelY = Text12_GetRowInline() * 0xC;
    return Text12_PutChar(ch);
}

// Upstream nonmatch: https://decomp.me/scratch/o7OVy
static inline void Text8_SyncPixelCursorB()
{
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
}

static inline s32 Text8_GetPixelXInTile()
{
    return gEwramData->text8PixelX & 7;
}

/**
 * @brief Return the value unchanged when bits 5..7 are clear; otherwise render the proportional eight-row glyph, advance/wrap the cursor and return -1.
 */
s32 Text8_PutChar(u16 ch)
{
    s32 *out;
    s32 mask;
    u32 pixelX;
    s32 i;
    s32 *dst;
    s32 width;
    s32 skip;
    s32 keepMask;
    s32 glyphIndex;
    u32 sh;

    if (!(ch & 0xE0))
    {
        return ch;
    }

    glyphIndex = ch - 0x20;
    skip = sText8GlyphMetrics[glyphIndex].sourceBitSkip;
    width = sText8GlyphMetrics[glyphIndex].width;
    if (((gEwramData->text8PixelX & 7) + width) <= 7)
    {
        pixelX = Text8_GetPixelXInTile();
        Text8_BuildGlyphRows(ch, skip, width);
        dst = (void*)gEwramData->textTileBuffer + ((gEwramData->text8PixelX >> 3) << 5) + (gEwramData->text8Row << 0xA);
        mask = 0;
        for (i = pixelX; i < (pixelX + width); i++)
        {
            mask |= 0xF << (i * 4);
        }
        keepMask = ~mask;

        sh = pixelX * 4;
        out = dst;
        for (i = 0; i < 8; i++)
        {
            *out++ = (*out & keepMask) | ((gTextGlyphRows[i] << sh) & mask);
        }
    }
    else
    {
        Text8_BlitGlyphAcrossColumns(ch, skip, width);
    }

    gEwramData->text8ColumnPixelOffset += width;
    if (gEwramData->text8ColumnPixelOffset > 5)
    {
        gEwramData->text8ColumnPixelOffset -= 6;

        gEwramData->text8Column += 1;
        if (gEwramData->text8Column > gEwramData->text8LastColumn)
        {
            gEwramData->text8Column = gEwramData->text8RegionLeft;

            gEwramData->text8Row += 1;
            if (gEwramData->text8Row > gEwramData->text8LastRow)
            {
                gEwramData->text8Row = gEwramData->text8RegionTop;
            }
            Text8_SyncPixelCursorB();
        }
        else
        {
            gEwramData->text8PixelX += width;
        }
    }
    else
    {
        gEwramData->text8PixelX += width;
    }
    return -1;
}

/**
 * @brief Set cursor column/row clamped to region upper bounds, clear the within-cell offset and render one value through Text8_PutChar.
 */
s32 Text8_PutCharAt(u16 column, u16 row, u16 ch)
{
    if (column > gEwramData->text8LastColumn)
    {
        column = gEwramData->text8LastColumn;
    }

    if (row > gEwramData->text8LastRow)
    {
        row = gEwramData->text8LastRow;
    }

    gEwramData->text8Column = column;
    gEwramData->text8Row = row;

    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
    return Text8_PutChar(ch);
}

/**
 * @brief Clear six pixels at the current twelve-row cursor through the appropriate tile-column path, then advance and wrap the cursor.
 */
void Text12_ClearNextCell(void)
{
    if (((gEwramData->text12PixelX & 7) + 6) <= 7)
    {
        Text12_ClearWithinColumn(6);
    }
    else
    {
        Text12_ClearAcrossColumns(6);
    }

    gEwramData->text12ColumnPixelOffset += 6;
    if (gEwramData->text12ColumnPixelOffset > 5)
    {
        gEwramData->text12ColumnPixelOffset -= 6;
        gEwramData->text12Column += 1;
        if (gEwramData->text12Column > gEwramData->text12LastColumn)
        {
            gEwramData->text12Column = gEwramData->text12RegionLeft;
            gEwramData->text12Row += 1;
            if (gEwramData->text12Row > gEwramData->text12LastRow)
            {
                gEwramData->text12Row = gEwramData->text12RegionTop;
            }
            // gEwramData->text12PixelY = Text12_GetRowInline() * 0xC;
            // gEwramData->text12ColumnPixelOffset = 0;
            // gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
            Text12_SyncPixelCursor();
        }
        else
        {
            gEwramData->text12PixelX += 6;
        }
    }
    else
    {
        gEwramData->text12PixelX += 6;
    }
}

/**
 * @brief Set the cursor to the supplied column/row and clear a six-pixel-wide cell, advancing the cursor through the existing clear helper.
 */
void Text12_ClearCell(u16 column, u16 row)
{
    gEwramData->text12Column = column;
    gEwramData->text12Row = row;
    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
    gEwramData->text12PixelY = Text12_GetRowInline() * 0xC;
    Text12_ClearNextCell();
}

/**
 * @brief Clear the configured region including the original extra boundary column, then reset the cursor to the region start.
 */
void Text12_ClearRegion(void)
{
    u16 i;
    u16 j;

    for (i = gEwramData->text12RegionTop; i <= gEwramData->text12LastRow; i++)
    {
        for (j = gEwramData->text12RegionLeft; j <= (gEwramData->text12LastColumn + 1); j++)
        {
                Text12_ClearCell(j, i);
        }
    }

    gEwramData->text12Column = gEwramData->text12RegionLeft;
    gEwramData->text12Row = gEwramData->text12RegionTop;
    gEwramData->text12ColumnPixelOffset = 0;
    gEwramData->text12PixelX = Text12_GetColumnInline() * 6;
    gEwramData->text12PixelY = Text12_GetRowInline() * 0xC;
}

static inline void Text8_SyncPixelCursor()
{
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
}

/**
 * @brief Clear six pixels at the current eight-row cursor through the appropriate tile-column path, then advance and wrap the cursor.
 */
void Text8_ClearNextCell(void)
{
    if (((gEwramData->text8PixelX & 7) + 6) <= 7)
    {
        Text8_ClearWithinColumn(6);
    }
    else
    {
        Text8_ClearAcrossColumns(6);
    }

    gEwramData->text8ColumnPixelOffset += 6;
    if (gEwramData->text8ColumnPixelOffset > 5)
    {
        gEwramData->text8ColumnPixelOffset -= 6;
        gEwramData->text8Column += 1;
        if (gEwramData->text8Column > gEwramData->text8LastColumn)
        {
            gEwramData->text8Column = gEwramData->text8RegionLeft;
            gEwramData->text8Row += 1;
            if (gEwramData->text8Row > gEwramData->text8LastRow)
            {
                gEwramData->text8Row = gEwramData->text8RegionTop;
            }
            Text8_SyncPixelCursor();
        }
        else
        {
            gEwramData->text8PixelX += 6;
        }
    }
    else
    {
        gEwramData->text8PixelX += 6;
    }
}

/**
 * @brief Set the cursor to the supplied column/row and clear a six-pixel-wide cell, advancing the cursor through the existing clear helper.
 */
void Text8_ClearCell(u16 column, u16 row)
{
    gEwramData->text8Column = column;
    gEwramData->text8Row = row;
    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
    Text8_ClearNextCell();
}

/**
 * @brief Clear the configured region including the original extra boundary column, then reset the cursor to the region start.
 */
void Text8_ClearRegion(void)
{
    u16 i;
    u16 j;

    for (i = gEwramData->text8RegionTop; i <= gEwramData->text8LastRow; i++)
    {
        for (j = gEwramData->text8RegionLeft; j <= (gEwramData->text8LastColumn + 1); j++)
        {
            Text8_ClearCell(j, i);
        }
    }

    gEwramData->text8Column = gEwramData->text8RegionLeft;
    gEwramData->text8Row = gEwramData->text8RegionTop;
    gEwramData->text8ColumnPixelOffset = 0;
    gEwramData->text8PixelX = Text8_GetColumnInline() * 6;
    gEwramData->text8PixelY = Text8_GetRowInline() * 8;
}

/**
 * @brief Store the destination tile-buffer pointer used by both text renderers.
 */
void Text_SetTileBuffer(void *buffer)
{
    gEwramData->textTileBuffer = buffer;
}

/**
 * @brief Store the glyph foreground color value used by both text renderers.
 */
void Text_SetForegroundColor(s32 color)
{
    gEwramData->textForegroundColor = color;
}

/**
 * @brief Store the background color value replicated into the glyph scratch rows; cell clearing still writes zero.
 */
void Text_SetBackgroundColor(s32 color)
{
    gEwramData->textBackgroundColor = color;
}

/**
 * @brief Render bytes after the two-byte header from the supplied offset until a control value is returned; return that control byte offset.
 */
s32 Text12_Print(void *text, s32 index)
{
    u8 *chars;

    chars = text + 2;
    while (Text12_PutChar(chars[index++]) == -1);

    return index - 1;
}

/**
 * @brief Render bytes after the two-byte header from the supplied offset until a control value is returned; return that control byte offset.
 */
s32 Text8_Print(void *text, s32 index)
{
    u8 *chars;

    chars = text + 2;
    while (Text8_PutChar(chars[index++]) == -1);

    return index - 1;
}

// TODO: rest of functions are inlines?

/**
 * @brief Return the current text pixel X coordinate.
 */
u16 Text12_GetPixelX(void)
{
    return gEwramData->text12PixelX;
}

/**
 * @brief Return the current text pixel X coordinate.
 */
u16 Text8_GetPixelX(void)
{
    return gEwramData->text8PixelX;
}

/**
 * @brief Return the current text column.
 */
u16 Text12_GetColumn(void)
{
    return gEwramData->text12Column;
}

/**
 * @brief Return the current text column.
 */
u16 Text8_GetColumn(void)
{
    return gEwramData->text8Column;
}

/**
 * @brief Return the pixel offset within the current six-pixel text cell.
 */
u16 Text12_GetCellOffset(void)
{
    return gEwramData->text12ColumnPixelOffset;
}

/**
 * @brief Return the pixel offset within the current six-pixel text cell.
 */
u16 Text8_GetCellOffset(void)
{
    return gEwramData->text8ColumnPixelOffset;
}

/**
 * @brief Return the current text pixel Y coordinate.
 */
u16 Text12_GetPixelY(void)
{
    return gEwramData->text12PixelY;
}

/**
 * @brief Return the current text pixel Y coordinate.
 */
u16 Text8_GetPixelY(void)
{
    return gEwramData->text8PixelY;
}

/**
 * @brief Return the current text row.
 */
u16 Text12_GetRow(void)
{
    return gEwramData->text12Row;
}

/**
 * @brief Return the current text row.
 */
u16 Text8_GetRow(void)
{
    return gEwramData->text8Row;
}

/**
 * @brief Return the length (end - start) of entry n of sText8GlyphMetrics.
 */
s32 MenuText_GetRangeLength(s32 index)
{
    return sText8GlyphMetrics[index].width - sText8GlyphMetrics[index].sourceBitSkip;
}

extern u32 sTextEntries[];

/**
 * @brief Return the indexed language-table entry with offsets 0, 0x3C4 or 0x788 according to language; retain the original unchecked indexing.
 */
u32 Text_GetLocalizedEntry(s32 index)
{
    s32 offset = 0;
    switch (gEwramData->gameState.language)
    {
    case 1: break;
    case 2: offset = 0x3C4; break;
    case 3: offset = 0x788; break;
    }
    return sTextEntries[index + offset];
}
