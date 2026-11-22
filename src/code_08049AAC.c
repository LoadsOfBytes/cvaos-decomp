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
 * @brief For the red or blue category row under the cursor, draw the label tiles
 * and the table value of the equipped soul (blue souls also draw their
 * four-tile row label); yellow or an empty slot draws no number.
 */
void SoulMenu_DrawEquippedSoulValue(struct SoulMenu *menu)
{
    struct PlayerStatus *equip;
    s32 value;
    s32 skip;
    s32 row;
    s32 cur;

    equip = &gEwramData->playerStatus;
    value = 0;
    skip = 0;
    HudMap_ClearTiles(1, 0x10, 4);
    HudMap_ClearTiles(1, 0x11, 4);
    HudMap_ClearTiles(1, 0x12, 4);
    cur = menu->cursor;
    if (cur == 2)
        return;
    if (menu->cursor == 0)
        goto case0;
    if (menu->cursor == 1)
        goto case1;
    goto tail;
case0:
    if (equip->equippedRedSoul == 0)
        skip = 1;
    else
    {
    value = sSomaRedSoulInfo[equip->equippedRedSoul].value;
    FillRow((u16 *)0x0600E442, 0x33AB);
    HudMap_ClearTiles(1, 0x12, 4);
    }
    goto tail;
case1:
    if (equip->equippedBlueSoul == 0)
        skip = 1;
    else
    {
    value = sBlueSoulInfo[equip->equippedBlueSoul].value;
    row = sBlueSoulInfo[equip->equippedBlueSoul].row >> 1;
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
    }
tail:
    if (skip == 0)
        HudMap_PrintNumber(4, 0x10, value, 3, 3, skip);
}
