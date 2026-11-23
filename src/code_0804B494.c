/* Item list page. */
#include "code_08009A0.h"
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

struct ItemInfo
{
    u16 name;
    u8 oneBasedIconId;
    u8 pad_3[0x10 - 0x3];
};

extern const struct ItemInfo sConsumableInfo[];
extern const u16 sItemNameTextIds[];
extern s32 HudMap_PrintPaddedNumber(s32 x, s32 y, s32 value, s32 digits);
extern void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count);
extern void MenuList_DrawScrollArrows(struct MenuPage *page, s32 raised);

/**
 * @brief 4B494 | Draw the item list page (32 items, 8 rows): name and count
 * of each owned item from the page top, blank rows, store the number of
 * entries and the item under the cursor. Declaration order i, idx, row
 * gives the original stack slot order of the loop temporaries.
 */
void ItemMenu_DrawList(struct PlayerStatus *stats, struct MenuPage *page)
{
    s32 i;
    s32 idx;
    s32 row;

    row = 0;
    idx = 0;
    page->item = 0xFF;
    for (i = 0; idx < page->top && i < 0x20; i++)
    {
        if (stats->itemInventory[i] != 0)
            idx++;
    }
    for (; row <= 7 && i < 0x20; i++)
    {
        if (stats->itemInventory[i] != 0)
        {
            const struct ItemInfo *info;
            s32 limit; /* never initialised, as in the original */
            s32 odd;
            u16 yy;
            s32 x;

            if (idx == page->cursor)
                page->item = i;
            info = &sConsumableInfo[i];
            while (1)
            {
                s32 v = *(vu16 *)0x04000006;
                if (v <= 0x40 || v >= limit)
                    break;
            }
            odd = row & 1;
            x = odd ? 0x15 : 0;
            yy = (row >> 1) + 0xC;
            Text8_SetRegion(x, yy, 0xE, 1);
            Text8_ClearRegion();
            Text8_SetPixelCursor((row & 1) << 7, yy);
            Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[info->name]), 0);
            HudMap_PrintPaddedNumber(odd ? 0x1C : 0xD, (row >> 1) + 0xB, stats->itemInventory[i], 1);
            idx++;
            row++;
        }
    }
    for (; row <= 7; row++)
    {
        s32 odd;
        s32 half;
        s32 x;

        odd = row & 1;
        x = odd ? 0x15 : 0;
        half = row >> 1;
        Text8_SetRegion(x, half + 0xC, 0xE, 1);
        Text8_ClearRegion();
        HudMap_ClearTilesLeft(odd ? 0x1C : 0xD, half + 0xB, 2);
    }
    for (; i < 0x20; i++)
    {
        if (stats->itemInventory[i] != 0)
            idx++;
    }
    page->count = idx;
    if (idx == 0)
    {
        Text8_SetRegion(0, 0xC, 0xA, 1);
        Text8_ClearRegion();
        Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
        page->top = idx;
        page->cursor = idx;
        page->item = 0xFF;
    }
    MenuList_DrawScrollArrows(page, 0);
}
