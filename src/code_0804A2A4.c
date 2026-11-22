/* Equipment item list page. */
#include "gba.h"
#include "structs/ewram.h"
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

struct Item20
{
    u16 name;
    u8 pad_2[6];
    u8 type;
    u8 pad_9[0x14 - 0x9];
};

struct Item28
{
    u16 name;
    u8 pad_2[0x1C - 0x2];
};

extern const struct Item28 sWeaponInfo[];
extern const struct Item20 sArmorInfo[];
extern const u16 sItemNameTextIds[];
extern void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count);
extern s32 HudMap_PrintPaddedNumber(s32 x, s32 y, s32 value_, s32 digits);

/**
 * @brief 4A2A4 | Draw the item list of an equipment slot (0 weapon,
 * 1 armor, 2 accessory): an optional "none" row on the first page, the
 * equipped items, then the owned items with their counts (8 rows), and
 * store the number of entries and the item under the cursor in page.
 */
void EquipmentMenu_DrawList(struct PlayerStatus *stats, struct MenuPage *page, s32 slot)
{
    const struct Item20 *table;
    s32 count;
    s32 type;
    u8 *owned;
    u8 *equipped;
    s32 row;
    s32 idx;
    s32 k;
    s32 i;
    s32 j;
    s32 found;
    s32 slots;
    u16 yy;
    const u16 *info;

    /* kept in a variable: it gets no register, reload substitutes the
     * constant (cmp #4 / blt and unfolded loop guards as in the original) */
    slots = 4;
    row = 0;
    page->item = 0xFF;
    switch (slot)
    {
    case 0:
        table = (const struct Item20 *)sWeaponInfo;
        count = 0x3B;
        type = 2;
        owned = (u8 *)stats + 0x58;
        equipped = (u8 *)stats + 0x174;
        break;
    case 1:
        table = sArmorInfo;
        count = 0x2D;
        type = 0;
        owned = (u8 *)stats + 0x93;
        equipped = (u8 *)stats + 0x178;
        break;
    case 2:
        table = sArmorInfo;
        count = 0x2D;
        type = 1;
        owned = (u8 *)stats + 0x93;
        equipped = (u8 *)stats + 0x17C;
        break;
    }
    if (page->top == 0)
    {
        Text8_SetRegion(0, 10, 13, 1);
        Text8_ClearRegion();
        Text8_Print(Text_GetLocalizedEntry(0x34F), 0);
        HudMap_ClearTilesLeft((row & 1) ? 0x1B : 0xD, (row >> 1) + 0xB, 2);
        row = 1;
    }
    else
        row = 0;
    idx = 1;
    page->item = 0xFF;
    k = 0;
    for (; idx < page->top && k < slots; k++)
    {
        if (equipped[k] != 0xFF && owned[equipped[k]] != 0)
            idx++;
    }
    for (i = 0; idx < page->top && i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
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
        s32 odd;
        s32 x;

        if (equipped[k] != 0xFF && owned[equipped[k]] != 0 && (slot == 0 || table[equipped[k]].type == type))
        {
            if (idx == page->cursor)
                page->item = equipped[k];
            if (slot != 0)
                info = &table[equipped[k]].name;
            else
                info = &sWeaponInfo[equipped[k]].name;
            do
                odd = row & 1;
            while (0); /* register priority */
            x = odd ? 0x15 : 0;
            yy = ((row + 4) >> 1) + 8;
            Text8_SetRegion(x, yy, 13, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor((row & 1) << 7, yy);
            Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[*info]), 0);
            HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, (row >> 1) + 0xB, owned[equipped[k]], 1);
            idx++;
            row++;
        }
    }
    for (; row <= 7 && i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
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
                s32 odd;
                s32 x;

                if (idx == page->cursor)
                    page->item = i;
                do /* register priority */
                {
                    if (slot != 0)
                        info = &table[i].name;
                    else
                        info = &sWeaponInfo[i].name;
                } while (0);
                odd = row & 1;
                x = odd ? 0x15 : 0;
                yy = ((row + 4) >> 1) + 8;
                Text8_SetRegion(x, yy, 13, 1);
                Text8_ClearRegion();
                if (odd)
                    Text8_SetPixelCursor((row & 1) << 7, yy);
                Text8_Print(Text_GetLocalizedEntry(sItemNameTextIds[*info]), 0);
                HudMap_PrintPaddedNumber(odd ? 0x1B : 0xD, (row >> 1) + 0xB, owned[i], 1);
                idx++;
                row++;
            }
        }
    }
    for (; row <= 7; row++)
    {
        Text8_SetRegion((row & 1) << 4, (row >> 1) + 10, 0x14, 1);
        Text8_ClearRegion();
        HudMap_ClearTilesLeft((row & 1) ? 0x1B : 0xD, (row >> 1) + 0xB, 2);
    }
    for (; i < count; i++)
    {
        if (owned[i] != 0 && (slot == 0 || table[i].type == type))
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
    }
    page->count = idx;
    if (idx == 1)
        page->cursor = 0;
}
