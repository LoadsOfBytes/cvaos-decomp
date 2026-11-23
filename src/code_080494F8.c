/* Soul menu: list of souls of one type (red/blue/yellow) - none row, equipped, owned, empty fill. */
#include "gba.h"
#include "structs/ewram.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
    u8 item;
};

extern void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count);
extern s32 HudMap_PrintPaddedNumber(s32 x, s32 y, s32 value_, s32 digits);
extern void MenuList_DrawScrollArrows(struct MenuPage *page, s32 raised);

/**
 * @brief 494F8 | Draw the soul list of soul type slot (red, blue, yellow):
 * an optional "none" row on the first page, the equipped souls, then the
 * owned souls with their counts (8 rows); store the number of entries and
 * the soul under the cursor in page.
 */
void SoulMenu_DrawList(struct MenuPage *page, s32 slot)
{
    u8 total;
    struct EwramData *ew;
    s32 type;
    s32 count;
    s32 textBase;
    u8 *equipped;
    s32 row;
    s32 idx;
    s32 k;
    s32 i;
    s32 j;
    s32 found;
    s32 slots;

    ew = gEwramData;
    slots = 4;
    row = 0;
    idx = 0;
    page->item = idx;
    switch (slot)
    {
    case 0:
        type = 0;
        count = 0x38;
        textBase = 0xE3;
        equipped = (u8 *)ew + 0x133DC;
        break;
    case 1:
        type = 1;
        count = 0x19;
        textBase = 0x11B;
        equipped = (u8 *)ew + 0x133E0;
        break;
    case 2:
        type = 2;
        count = 0x24;
        textBase = 0x133;
        equipped = (u8 *)ew + 0x133E4;
        break;
    }
    if (page->top == 0)
    {
        s32 x;
        s32 odd;
        s32 half;

        odd = row & 1;
        x = odd ? 0x15 : 0;
        half = row >> 1;
        Text8_SetRegion(x, half + 0xC, 0xE, 1);
        Text8_ClearRegion();
        Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
        HudMap_ClearTilesLeft(odd ? 0x1B : 0xD, half + 0xA, 1);
        row = 1;
    }
    else
        row = 0;
    idx = 1;
    page->item = 0;
    k = 0;
    for (; idx < page->top && k < slots; k++)
    {
        if (equipped[k] != 0xFF && SoulInventory_GetSoulTotal(type, equipped[k] - 1) != 0)
            idx++;
    }
    for (i = 1; idx < page->top && i < count; i++)
    {
        if (SoulInventory_GetSoulTotal(type, i - 1) != 0)
        {
            idx++;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    idx--;
            }
        }
    }
    for (; row <= 7 && k < slots; k++)
    {
        if (equipped[k] != 0xFF && (total = SoulInventory_GetSoulTotal(type, equipped[k] - 1)) != 0)
        {
            s32 x;
            u16 yy;
            s32 odd;
            s32 half;

            if (idx == page->cursor)
                page->item = equipped[k];
            odd = row & 1;
            x = odd ? 0x15 : 0;
            half = row >> 1;
            yy = half + 0xC;
            Text8_SetRegion(x, yy, 0xE, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor((row & 1) << 7, yy);
            Text8_Print(Text_GetLocalizedEntry(textBase + equipped[k] - 1), 0);
            HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, half + 0xA, total, 1);
            idx++;
            row++;
        }
    }
    for (; row <= 7 && i < count; i++)
    {
        if ((total = SoulInventory_GetSoulTotal(type, i - 1)) != 0)
        {
            s32 dup;

            dup = 0;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    dup = 1;
            }
            if (!dup)
            {
                s32 x;
                s32 odd;
                s32 half;

                if (idx == page->cursor)
                    page->item = i;
                odd = row & 1;
                x = odd ? 0x15 : 0;
                Text8_SetRegion(x, ((row + 8) >> 1) + 8, 0xE, 1);
                Text8_ClearRegion();
                half = row >> 1;
                if (odd)
                    Text8_SetPixelCursor((row & 1) << 7, half + 0xC);
                Text8_Print(Text_GetLocalizedEntry(textBase + i - 1), 0);
                HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, half + 0xA, total, 1);
                idx++;
                row++;
            }
        }
    }
    if (row > 7)
        goto fill_end;
fill:
    {
        s32 x;
        s32 odd;
        s32 half;

        odd = row & 1;
        x = odd ? 0x15 : 0;
        half = row >> 1;
        Text8_SetRegion(x, half + 0xC, 0xE, 1);
        Text8_ClearRegion();
        HudMap_ClearTilesLeft(odd ? 0x1B : 0xD, half + 0xA, 1);
        do { row++; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
    }
    if (row <= 7)
        goto fill;
fill_end:
    while (i < count)
    {
        if (SoulInventory_GetSoulTotal(type, i - 1) != 0)
        {
            found = 0;
            for (j = 0; j < slots; j++)
            {
                if (equipped[j] != 0xFF && i == equipped[j])
                    found = 1;
            }
            if (!found)
                idx++;
        }
        i++;
    }
    page->count = idx;
    MenuList_DrawScrollArrows(page, 1);
}
