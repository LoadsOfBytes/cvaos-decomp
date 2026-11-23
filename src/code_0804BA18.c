/* Ability soul page. */
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
};

extern void MenuPage_Reset(void *page);
extern void Text12_SetupTileRegion(s32 x, s32 y, u16 tile, s32 width, s32 height);
extern void AbilityMenu_PrintSelectedName(struct MenuPage *page);
extern void AbilityMenu_DrawMarkers(struct MenuPage *page);
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
extern void Cursor_SetTilePosition(s32 x, s32 y);
extern void Cursor_Draw(void);
extern void MenuPageTitle_Create(void *entity, s32 type);

/**
 * @brief 4BA18 | Open the ability soul page: frame, six rows of names
 * (owned ability souls by name, others as "-----") and the cursor.
 */
void AbilityMenu_Open(struct MenuPage *page)
{
    s32 i;

    MenuPage_Reset(page);
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        page->count = 6;
    while (0);
    Text12_SetupTileRegion(4, 0xD, 0x3200, 0x1F, 4);
    AbilityMenu_PrintSelectedName(page);
    for (i = 0; i < page->count >> 1; i++)
    {
        HudMap_WriteTiles(3, i * 2 + 4, i * 0x20 + 0x3380, 10);
        HudMap_WriteTiles(0x11, i * 2 + 4, i * 0x20 + 0x3390, 10);
    }
    for (i = 0; i < page->count; i++)
    {
        s32 odd;
        s32 x;
        u16 yy;

        odd = i & 1;
        x = odd ? 0x15 : 0;
        yy = (i >> 1) + 0xC;
        Text8_SetRegion(x, yy, 0xE, 1);
        Text8_ClearRegion();
        if (odd)
            Text8_SetPixelCursor(0x80, yy);
        if (SoulInventory_GetSoulTotal(3, i))
            Text8_Print(Text_GetLocalizedEntry(i + 0x156), 0);
        else
            Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
    }
    Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 2, (page->cursor >> 1) * 2 + 4);
    Cursor_Draw();
    MenuPageTitle_Create((u8 *)gEwramData + 0xF34, 0x22);
    AbilityMenu_DrawMarkers(page);
}
