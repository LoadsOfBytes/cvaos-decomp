#include "gba.h"
#include "structs/ewram.h"

struct SoulMenu
{
    u8 pad[0x13];
    u8 type;
    u8 count;
    s8 cursor;
    u8 pad16;
    u8 selected;
};

struct Tbl16 { u8 pad[6]; s16 value; u8 pad2[8]; };
extern const struct Tbl16 sSomaRedSoulInfo[];
struct Tbl12 { u8 pad[4]; u8 value; s8 row; u8 pad2[6]; };
extern const struct Tbl12 sBlueSoulInfo[];

extern void HudMap_ClearTiles(s32 a, s32 b, s32 c);
extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

static inline void FillRow(u16 *dst, s32 tile)
{
    s32 i;
    for (i = 3; i >= 0; i--)
    {
        *dst = tile;
        dst++;
        tile++;
    }
}

/**
 * @brief As SoulMenu_DrawEquippedSoulValue for the soul selected in the open
 * red/blue list; nothing when the cursor is on the first row or for yellow.
 */
void SoulMenu_DrawSelectedSoulValue(struct SoulMenu *menu)
{
    s32 value;
    s32 i;
    s32 row;
    u16 *dst;
    s32 tile;

    value = 0;
    HudMap_ClearTiles(1, 0x10, 4);
    HudMap_ClearTiles(1, 0x11, 4);
    HudMap_ClearTiles(1, 0x12, 4);
    if (menu->cursor == 0)
        return;
    if (menu->type == 2)
        return;
    if (menu->type == 0)
        goto case0;
    if (menu->type == 1)
        goto case1;
    goto tail;
case0:
        value = sSomaRedSoulInfo[menu->selected].value;
        FillRow((u16 *)0x0600E442, 0x33AB);
        HudMap_ClearTiles(1, 0x12, 4);
    goto tail;
case1:
        value = sBlueSoulInfo[menu->selected].value;
        row = sBlueSoulInfo[menu->selected].row >> 1;
        FillRow((u16 *)0x0600E442, 0x33BB);
        {
            u16 *dst = (u16 *)0x0600E482;
            s32 tile = row * 16 + 0x33CB;
            s32 i;
            for (i = 3; i >= 0; i--)
            {
                *dst = tile;
                dst++;
                tile++;
            }
        }
tail:
    HudMap_PrintNumber(4, 0x10, value, 3, 3, 0);
}
