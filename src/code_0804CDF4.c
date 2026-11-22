/* Bestiary page: enemy number, HP/EXP, weak/resist icons and soul icon. */
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    u8 pad_0[0xC];
    u16 hp;
    u8 pad_E[0x10 - 0xE];
    u16 exp;
    u8 pad_12[0x17 - 0x12];
    u8 soulType;
    u8 soulIndex; /* 1-based, 0 = none */
    u8 pad_19;
    u16 resist;
    u16 weak;
    u8 pad_1E[0x24 - 0x1E];
};

struct MenuPage
{
    u8 pad_0[0x13];
    u8 descriptionMode;
    u8 count;
    u8 cursor;
    u8 top;
};

extern const struct EnemyTemplateView sEnemyData[];
/* tile as s32: the header u16 would add a zero extension of the argument */
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern void HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 color, s32 flag);
extern s32 BestiaryMenu_DrawDescription(s32 id, s32 mode);
extern void BestiaryMenu_PrintDrops(struct MenuPage *page);
extern void BestiaryMenu_DrawDetailTitle(struct MenuPage *page);
extern void BestiaryMenu_DrawAttributeIcon(s32 x, s32 y, s32 type);

/**
 * @brief Draw the enemy entry under the cursor: number, HP, EXP, drop names, detail
 * title, description, weak/resist icons and soul icon.
 */
void BestiaryMenu_DrawEntry(struct MenuPage *page)
{
    const struct EnemyTemplateView *entry;
    s32 num;
    s32 base;
    s32 x;
    s32 y;
    s32 n;
    s32 i1, i2, i3, i4;
    s32 tile;
    s32 k;
    s32 pal;
    s32 sx;
    s32 sy;
    s32 y2;

    entry = &sEnemyData[page->cursor];
    num = page->cursor + 1;
    if (num <= 0x63)
    {
        HudMap_ClearTiles(6, 1, 1);
        HudMap_PrintNumber(8, 1, num, 2, 1, 1);
    }
    else
    {
        HudMap_PrintNumber(8, 1, num, 3, 1, 1);
    }
    HudMap_PrintNumber(0x14, 2, entry->hp, 4, 1, 0);
    HudMap_PrintNumber(0x1B, 0xB, entry->exp, 8, 1, 0);
    BestiaryMenu_PrintDrops(page);
    BestiaryMenu_DrawDetailTitle(page);
    BestiaryMenu_DrawDescription((s8)page->cursor, page->descriptionMode);

    base = 0x15;
    x = base;
    y = 4;
    n = 0;
    for (i1 = 0; i1 <= 8; i1++)
    {
        if (!((entry->weak >> i1) & 1))
            n++;
        else
        {
            BestiaryMenu_DrawAttributeIcon(x++, y, i1);
            if (x - base == 5)
            {
                y++;
                x = base;
            }
        }
    }
    do
        y2 = 6;
    while (0);
    for (i2 = 0; i2 < n; i2++)
    {
        HudMap_ClearTiles(x++, y, 1);
        if (x - base == 5)
        {
            y++;
            x = base;
        }
    }
    x = base;
    y = y2;
    n = 0;
    for (i3 = 0; i3 <= 8; i3++)
    {
        if (!((entry->resist >> i3) & 1))
            n++;
        else
        {
            BestiaryMenu_DrawAttributeIcon(x++, y, i3);
            if (x - base == 5)
            {
                y++;
                x = base;
            }
        }
    }
    for (i4 = 0; i4 < n; i4++)
    {
        HudMap_ClearTiles(x++, y, 1);
        if (x - base == 5)
        {
            y++;
            x = base;
        }
    }

    sx = 0x18;
    sy = 1;
    k = 0xB;
    pal = 0x300;

    if (entry->soulType == 0 && entry->soulIndex == 0)
    {
        HudMap_ClearTiles(0x18, 2, 3);
        tile = 0x309;
        HudMap_WriteTiles(0x19, 1, tile + 0x90, 2);
        tile += 0xA0;
        HudMap_WriteTiles(0x19, 2, tile, 2);
    }
    else if (SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1))
    {
        {
            /* otherwise local-alloc folds pal + 8 via REG_EQUIV */
            register s32 p_ asm("r2") = pal; /* FAKEMATCH: pinned register */
            tile = p_ + 8;
        }
        HudMap_WriteTiles(sx, sy, tile + (k << 4), 3);
        HudMap_WriteTiles(sx, sy + 1, tile += (k + 1) << 4, 3);
    }
    else
    {
        HudMap_ClearTiles(sx, sy, 3);
        HudMap_ClearTiles(sx, sy + 1, 3);
    }
}
