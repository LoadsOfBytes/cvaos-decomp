#include "code/code_08039340.h"

struct ButtonConfigPageView
{
    u8 pad_0[0x18];
    u8 choices[4];
};

extern const u8 sButtonConfigChoiceSlots[4];
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);

static inline void MoveText(s32 n)
{
    gEwramData->text8ColumnPixelOffset += n;
    gEwramData->text8PixelX += n;
}

/**
 * @brief 4C264 | Draw the button configuration page: the four action
 * labels (texts 0x80..0x87) and the button currently assigned to each.
 */
void ButtonConfigMenu_Draw(struct ButtonConfigPageView *page)
{
    s32 i;
    s32 count;

    Text8_SetRegion(0, 5, 0x1E, 1);
    Text8_ClearRegion();
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    Text8_PutChar(0x80);
    Text8_PutChar(0x81);
    gEwramData->text8ColumnPixelOffset += 8;
    gEwramData->text8PixelX += 8;
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    Text8_PutChar(0x82);
    Text8_PutChar(0x83);
    gEwramData->text8ColumnPixelOffset += 8;
    gEwramData->text8PixelX += 8;
    gEwramData->text8ColumnPixelOffset += 6;
    gEwramData->text8PixelX += 6;
    Text8_PutChar(0x86);
    Text8_PutChar(0x87);
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    gEwramData->text8ColumnPixelOffset += 6;
    gEwramData->text8PixelX += 6;
    Text8_PutChar(0x84);
    Text8_PutChar(0x85);
    gEwramData->text8ColumnPixelOffset += 7;
    gEwramData->text8PixelX += 7;
    for (i = 0; i <= 3; i++)
    {
        count = 7;
        if (i == 1)
            count = 6;
        HudMap_WriteTiles(6, i * 2 + 4, i * 0x10 + 0x3359, count);
        HudMap_WriteTiles(0x12, i * 2 + 4, page->choices[sButtonConfigChoiceSlots[i]] * 3 + 0x32A0, 3);
    }
}
