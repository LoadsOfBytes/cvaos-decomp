/* Enemy list of the bestiary page.
 *
 * Compiler-sensitive constructs:
 * - case 0: the row-clearing loop is a guarded do-while counting k down from
 *   n.  The guard compares `z` (zero, set before the switch and again after
 *   it), which global-alloc leaves without a register, so reload
 *   rematerialises the zero (movs r0, #0; cmp r0, rN).  The constant 1 and
 *   the tile `t` are set after the guard in that order.
 * - `z = i = 0;` after the switch keeps `i` referenced last, so CSE keeps it
 *   as the head of the zero class: the case 1 guard compares i instead of j,
 *   j loses one reference and its priority drops below the constant 1
 *   (j in sl, 1 in sb).
 * - tail: `u` is separate from the tile variable (lives in r0); the m == 6
 *   address is written with `u + 0x100`, so CSE folds it into one
 *   u + 0x0600A118 whose constant reload puts into r4 (r4 joins the spill
 *   registers, which fixes the reload rotation of the whole function).
 * - the dead `odd = 14;` in the second tail loop adds one insn, so loop.c
 *   leaves the 0x10000 constant for its second pass (orrs before it). */
#include "structs/ewram.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "syscalls.h"

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    u8 pad_0[0x17];
    u8 soulType;
    u8 soulIndex; /* 1-based, 0 = none */
    u8 pad_19[0x24 - 0x19];
};

struct MenuPage
{
    u8 pad_0[0xA];
    u8 state;
    u8 subState;
    u8 stage;
    u8 timer;
    u8 pad_E[0x14 - 0xE];
    s8 count;
    s8 cursor;
    s8 top;
};

struct Unk_08118510
{
    s32 unk[4];
};

extern const struct EnemyTemplateView sEnemyData[];
extern const u16 sEnemyNameTextIds[];
extern const struct Unk_08118510 sUnk_08118510;
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count);
extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern s32 HudMap_PrintPaddedNumberInPalette(s32 x, s32 y, s32 value_, s32 digits, s32 palette);

static inline s32 IsSeen(s32 id)
{
    s32 v = gEwramData->playerStatus.enemyFlags[id >> 5] & (1 << (id & 0x1F));
    return (u32)(-v | v) >> 31;
}

/**
 * @brief Draw the enemy list: up to ten entries starting at the page of the
 * cursor (name, number), blank rows, the page counter and the header;
 * state 0 first clears the rows.
 */
void BestiaryMenu_DrawList(struct MenuPage *page)
{
    s32 n;
    s32 id;
    s32 i;
    s32 y;
    s32 j;
    s32 row;
    s32 k;
    s32 total;
    s32 odd;
    s32 t;
    s32 z;
    s32 one;

    page->top = (s8)(page->cursor / 10) * 10;
    id = page->top;
    n = page->count - id;
    if (n > 10)
        n = 10;
    z = 0;
    switch (page->stage)
    {
    case 0:
        i = 0;
        y = 5;

        if (z < n)
        {
            one = 1;
            t = -0xCE0;
            k = n;
            do
            {
                HudMap_WriteTiles((i & one) ? 0x14 : 6, y, t, 10);
                Text_SetForegroundColor(15);
                i++;
                if (!(i & one))
                    y += 2;
                t += 16;
            } while (--k != 0);
        }
        page->stage++;
    case 1:
        i = 0;
        y = 5;
        j = 0;
        row = 2;
        for (; j < n; j++, row++, id++)
        {
            const struct EnemyTemplateView *entry = &sEnemyData[id];
            struct Unk_08118510 unused = sUnk_08118510;
            u16 yy;
            s32 x;
            s32 text;
            s32 num;

            if ((SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1) != 0 && entry->soulIndex != 0)
                || entry->soulIndex == 0)
                Text_SetForegroundColor(15);
            else
                Text_SetForegroundColor(14);
            odd = row & 1;
            x = odd ? 0x15 : 0;
            yy = (row >> 1) + 8;
            Text8_SetRegion(x, yy, 14, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor(0x80, yy);
            if (IsSeen(id))
                text = sEnemyNameTextIds[id];
            else
            {
                Text_SetForegroundColor(14);
                text = 0x34E;
            }
            Text8_Print(Text_GetLocalizedEntry(text), 0);
            num = page->top + j + 1;
            if (num <= 99)
            {
                HudMap_ClearTiles((i & 1) ? 0x10 : 2, y, 1);
                HudMap_PrintNumber((i & 1) ? 0x12 : 4, y, page->top + j + 1, 2, 1, 1);
            }
            else
            {
                HudMap_PrintNumber((i & 1) ? 0x12 : 4, y, num, 3, 1, 0);
            }
            i++;
            if (!(i & 1))
                y += 2;
        }
        Text_SetForegroundColor(15);
        for (; j <= 9; j++, row++)
        {
            u16 yy;
            s32 x;

            odd = row & 1;
            x = odd ? 0x15 : 0;
            yy = (row >> 1) + 8;
            Text8_SetRegion(x, yy, 13, 1);
            Text8_ClearRegion();
            if (odd)
                Text8_SetPixelCursor(0x80, yy);
            HudMap_ClearTilesLeft((i & 1) ? 0x12 : 4, y, 3);
            i++;
            if (!(i & 1))
                y += 2;
        }
        HudMap_PrintPaddedNumberInPalette(0xD, 0x11, (s8)(page->top / 10) + 1, 2, 1);
        HudMap_WriteTiles(0xE, 0x11, 0x139B, 1);
        HudMap_PrintPaddedNumberInPalette(0x10, 0x11, (page->count - 1) / 10 + 1, 2, 1);
        HudMap_WriteTiles(2, 0x11, 0x174C, 1);
        HudMap_WriteTiles(0x1C, 0x11, 0x134C, 1);
        Text8_SetRegion(0, 0xF, 0x29, 1);
        Text8_SetPixelCursor(0x60, 0xF);
        Text8_PutChar(0x84);
        Text8_PutChar(0x85);
        gEwramData->text8ColumnPixelOffset += 9;
        gEwramData->text8PixelX += 9;
        Text8_PutChar(0x86);
        Text8_PutChar(0x87);
        HudMap_WriteTiles(3, 0x11, 0xF3EC, 1);
        HudMap_WriteTiles(4, 0x11, 0xF3ED, 1);
        HudMap_WriteTiles(0x1A, 0x11, 0xF3EE, 1);
        HudMap_WriteTiles(0x1B, 0x11, 0xF3EF, 1);
    }
    z = i = 0;
    Text8_SetRegion(0x15, 0xE, 0x12, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0x80, 0xE);
    Text8_Print(Text_GetLocalizedEntry(0x3B1), 0);
    total = SoulInventory_GetCompletionTenths();
    Text8_SetRegion(0, 0xF, 0x10, 1);
    Text8_ClearRegion();
    Text8_Print(Text_GetLocalizedEntry(0x3B2), z);
    {
        s32 m;
        u32 *p;
        s32 u;

        for (m = 0, odd = 14, p = (u32 *)0x0600BC80; m <= 7; p++, m++)
        {
            if (m == 2 || m == 5)
                *p = 0x10000;
            else
                *p = 0;
            u = odd << 9;
        }
        for (m = 0; m <= 7; m++)
        {
            u32 a;
            odd = 14;
            a = (u | 0x100) + m * 4;
            if (m == 6)
                *(u32 *)(0x0600A000 + (u + 0x100) + m * 4) = 0x10000;
            else
                *(u32 *)(0x0600A000 + a) = 0;
        }
    }
    HudMap_PrintNumber(0x18, 0xF, Div(total, 10), 3, 1, 0);
    HudMap_PrintNumber(0x1A, 0xF, Mod(total, 10), 1, 1, 1);
}
