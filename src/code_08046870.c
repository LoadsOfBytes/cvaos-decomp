/* Draw a time value (frames -> seconds, minutes, hours) as three two-digit numbers on the BG map. */
#include "types.h"
#include "bg_map.h"
#include "syscalls.h"

static inline u16 *MapAddr(s32 x, s32 y)
{
    s32 xo = x << 1;
    s32 yo = y << 6;
    return (u16 *)(xo + ({ s32 t_ = yo + BG_MAP_VRAM; t_; }));
}
static inline u16 *MapAddr2(s32 x, s32 y)
{
    s32 xo = x << 1;
    s32 yo = y << 6;
    return (u16 *)(xo + (yo + BG_MAP_VRAM));
}
static inline u16 *MapAddr3(s32 x, s32 y)
{
    s32 yo = y << 6;
    return (u16 *)((x << 1) + (yo + BG_MAP_VRAM));
}
static inline void DrawRow(s32 x, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    map = MapAddr(x, y);
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}
static inline void DrawRow2(s32 x, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    map = MapAddr2(x, y);
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}
/* Inlined copy of HudMap_PrintPaddedNumberInPalette (two digits). */
static inline s32 PrintNumber(s32 x, s32 y, s32 value_, s32 digits, s32 palette)
{
    u16 *map;
    s32 i;
    s32 value;
    s32 attr = (palette << 12) | 0x300;

    value = value_;

    if (value < 0)
    {
        DrawRow(x, y, attr, 1);
        return 1;
    }

    map = MapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        *map = attr + Mod(value, 10);
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

static inline s32 PrintNumberG(s32 x, s32 y, s32 value_, s32 digits, s32 palette)
{
    u16 *map;
    s32 i;
    s32 value;
    s32 attr = (palette << 12) | 0x300;

    value = value_;

    if (value < 0)
    {
        DrawRow(x, y, attr, 1);
        return 1;
    }

    map = MapAddr(x, y);
    i = 1;
    goto test;
    do
    {
        *map = attr + Mod(value, 10);
        value = Div(value, 10);
        map--;
        i++;
    test:;
    } while (i <= digits || value != 0);
    return i - 1;
}

void Menu_DrawPlayTime(s32 x, s32 y, s32 palette, s32 frames)
{
    frames = Div(frames, 60);

    PrintNumber(x, y, Mod(frames, 60), 2, palette);
    frames = Div(frames, 60);
    PrintNumberG(x - 3, y, Mod(frames, 60), 2, palette);
    PrintNumberG(x - 6, y, Div(frames, 60), 2, palette);
    /* FAKEMATCH: 16 empty asm insns (no code) raise gcse max_cuid 160 -> 176; the PRE expression hash table then has 89 buckets and the reaching regs (= spill slot order) come out y<<6, palette<<12, x-6 as in the target. */
    asm(""); asm(""); asm(""); asm(""); asm(""); asm(""); asm(""); asm(""); /* FAKEMATCH: padding of gcse max_cuid, see above */
    asm(""); asm(""); asm(""); asm(""); asm(""); asm(""); asm(""); asm(""); /* FAKEMATCH: padding of gcse max_cuid, see above */
}
