#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

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
    u8 pad_0[0x13];
    u8 descriptionMode;
    u8 count;
    u8 cursor;
    u8 top;
};

extern const struct EnemyTemplateView sEnemyData[];
extern const u16 sEnemyNameTextIds[];
extern void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count);
extern void HudMap_ClearTiles(s32 x, s32 y, s32 count);
extern s32 MenuText_Measure(s32 text, s32 mode);

/**
 * @brief 4D0D0 | Draw the title row of the bestiary detail view: soul name
 * centred at x = 0xB0 and, in name mode, the soul kind icon.
 * Note: total is used uninitialised when the entry has no soul, as in
 * the original.
 */
void BestiaryMenu_DrawDetailTitle(struct MenuPage *page)
{
    s32 id;
    const struct EnemyTemplateView *entry;
    s32 mode;
    u8 total;
    s32 text;
    s32 width;
    s32 col;
    s32 row;

    id = page->cursor;
    entry = &sEnemyData[id];
    mode = page->descriptionMode;
    if (entry->soulIndex == 0)
        mode = 0;
    else
        total = SoulInventory_GetSoulTotal(entry->soulType, entry->soulIndex - 1);
    if (total == 0 && mode == 1)
        mode = 0;
    HudMap_ClearTiles(2, 0xC, 0x14);
    text = sEnemyNameTextIds[id];
    width = MenuText_Measure(text, 1);
    HudMap_WriteTiles(2, 0xC, 0x1310, 0xC);
    Text8_SetRegion(0x15, 8, 0x12, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0xB0 - width / 2, 8);
    Text8_Print(Text_GetLocalizedEntry(text), 0);
    if (mode != 0)
    {
        switch (entry->soulType)
        {
        case 0:
            col = 0xD;
            row = 0xF;
            break;
        case 1:
            col = 0xC;
            row = 0xF;
            break;
        case 2:
            col = 0xB;
            row = 0xF;
            break;
        case 3:
            col = 8;
            row = 0xA;
            break;
        }
        HudMap_WriteTiles(2, 0xC, (row * 16 - 0x3000 + col) | 0x1300, 1);
    }
}
