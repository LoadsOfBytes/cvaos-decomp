/* Draw current/max HP and MP with their seven-tile menu bars. */
#include "structs/ewram.h"
#include "syscalls.h"

static inline void DrawTileRow(s32 x, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;
    s32 xo = x << 1;
    s32 yo = y << 6;

    map = (u16 *)(xo + ({ s32 t_ = yo + 0x0600E000; t_; }));
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}

static inline u16 *BgMapAddr(s32 x, s32 y)
{
    s32 xo = x << 1;
    s32 yo = y << 6;

    return (u16 *)(xo + ({ s32 t_ = yo + 0x0600E000; t_; }));
}

static inline s32 DrawStatNumber(s32 x, s32 y, s32 value, s32 digits, s32 attr, s32 zeroPad)
{
    u16 *map;
    s32 i;

    if (value < 0)
    {
        DrawTileRow(x, y, attr, 1);
        return 1;
    }
    map = BgMapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        s32 d = Mod(value, 10);

        if (i != 1 && value == 0 && zeroPad == 0)
            *map = attr + 13;
        else
            *map = attr + d;
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

static inline void DrawStatBar(s32 x, s32 y, s32 current, s32 maximum, s32 palette)
{
    s32 amount;
    u16 *map;
    s32 i;

    if (current > 0)
    {
        amount = (100 * current / maximum) * 0.56;
        if (amount == 0)
            amount = 1;
    }
    else
        amount = 0;
    map = BgMapAddr(x, y);
    for (i = 0; i < 7; i++)
    {
        if (i == amount / 8)
            *map = (0x338 - (amount - i * 8)) | (palette << 12);
        else if (amount >= (i + 1) * 8)
            *map = 0x330 | (palette << 12);
        else
            *map = 0x338 | (palette << 12);
        map++;
    }
}

void Menu_DrawHpMpBars(s32 x, s32 y)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;

    DrawStatNumber(x, y, player->currentHP, 4, 0x3300, 0);
    DrawStatNumber(x, y + 1, player->currentMP, 4, 0x3300, 0);
    DrawStatNumber(x + 5, y, player->maxHP, 4, 0x3300, 0);
    DrawStatNumber(x + 5, y + 1, player->maxMP, 4, 0x3300, 0);
    DrawStatBar(x + 7, y, player->currentHP, player->maxHP, 3);
    DrawStatBar(x + 7, y + 1, player->currentMP, player->maxMP, 4);
}
