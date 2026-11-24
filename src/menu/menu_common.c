/*
 * Menu helpers: grid cursor, tile rows, HP / MP bars, stats, play time, menu text, the HUD map numbers and the 16x16 tile block.
 */
#include "menu_common.h"
#include "types.h"
#include "bg_map.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "gameplay.h"
#include "macros.h"
#include "gba.h"
#include "text12.h"
#include "data/data_0E5364.h"

struct MenuGrid
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
    s8 top;
};

/**
 * @brief Move the cursor of a two-column menu grid with an eight-entry visible page:
 * left/right toggles the column, up/down moves one row, R/L moves one page.
 * Input priority and the signed byte fields are preserved.
 *
 * @return 0 nothing changed, 1 cursor moved, 2 the visible top changed
 */
s32 MenuGrid_MoveCursor(struct MenuGrid *page, s32 input)
{
    s32 oldTop = page->top;
    s32 result = 0;

    if (input & 0x30)
    {
        if (page->cursor + 1 < page->count || (page->cursor & 1))
        {
            page->cursor ^= 1;
            result = 1;
        }
    }
    else if (input & 0x40)
    {
        if (page->cursor > 1)
        {
            page->cursor -= 2;
            if (page->cursor < page->top)
                page->top -= 2;
            result = 1;
        }
    }
    else if (input & 0x80)
    {
        if (page->cursor < page->count - 1)
        {
            page->cursor += 2;
            if (page->cursor >= page->count)
                page->cursor = page->count - 1;
            if (page->cursor >= page->top + 8)
                page->top += 2;
            result = 1;
        }
    }
    else if (input & 0x100)
    {
        if (oldTop + 8 < page->count)
        {
            page->cursor += 8;
            page->top += 8;
            if (page->cursor > page->count - 1)
                page->cursor = page->count - 1;
            result = 1;
        }
        else if (page->cursor < page->count - 1)
        {
            page->cursor = page->count - 1;
            result = 1;
        }
    }
    else if (input & 0x200)
    {
        if (page->top > 7)
        {
            page->cursor -= 8;
            page->top -= 8;
            result = 1;
        }
        else if (page->cursor != 0)
        {
            if (page->cursor <= 7)
                page->cursor = 0;
            else
                page->cursor -= 8;
            page->top = 0;
            result = 1;
        }
    }

    if (page->top != oldTop)
        result = 2;
    return result;
}

/* Row writers at map column x: the constant part of the address
 * (VRAM + x * 2) is folded into one literal. */
static inline void DrawTileRowAt(s32 x, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    map = (u16 *)((y << 6) + (BG_MAP_VRAM + x * 2));
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}

static inline void FillRowAt(s32 x, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    map = (u16 *)((y << 6) + (BG_MAP_VRAM + x * 2));
    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/* Row writers at a byte offset xo into the map row. */
static inline void DrawTileRowOfs(s32 xo, s32 y, s32 tile, s32 count)
{
    u16 *map;
    u16 t = tile;
    s32 i;

    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(xo + y);
    for (i = 0; i < count; i++)
    {
        *map = t;
        t++;
        map++;
    }
}

static inline void FillRowOfs(s32 xo, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(xo + y);
    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/* Draw the scroll arrows of an item list: up arrow when scrolled down,
 * down arrow when more than 8 entries remain below.
 * xo = x * 2 sits after the first if/else; GCSE (PRE) copies it into both
 * branches, where it folds to 58, which is the shape of the original. */
void MenuList_DrawScrollArrows(struct EwramData_EntityData *page, s32 raised)
{
    s32 tile = 0xB34D;
    s32 y = 11;
    s32 x;
    s32 xo;

    if (raised != 0)
        y = 10;
    x = 29;
    if (page->ext.menuList.top > 1)
        DrawTileRowAt(x, y, tile, 1);
    else
        FillRowAt(x, y, 0x330D, 1);
    y += 3;
    tile++;
    xo = x * 2;
    if (page->ext.menuList.count - 8 > page->ext.menuList.top)
        DrawTileRowOfs(xo, y, tile, 1);
    else
        FillRowOfs(xo, y, 0x330D, 1);
}

/* Draw current/max HP and MP with their seven-tile menu bars. */
static inline void DrawTileRow2(s32 x, s32 y, s32 tile, s32 count)
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

static inline u16 *BgMapAddr2(s32 x, s32 y)
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
        DrawTileRow2(x, y, attr, 1);
        return 1;
    }
    map = BgMapAddr2(x, y);
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
    map = BgMapAddr2(x, y);
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

/* Status screen: draw the four current stats (STR, CON, INT, LCK) as
 * right-aligned numbers (clamped to 999; a negative value draws one blank tile). */
void Menu_DrawCurrentStats(s32 x, s32 y)
{
    s16 *stat = gEwramData->playerState.currentStats;
    s32 i;
    for (i = 0; i <= 3; i++, stat++)
    {
        s32 col = x * 2;
        s32 v = *stat;
        s32 value;
        s32 row;
        u16 base;
        if (v > 1000)
            v = 999;
        row = y + i;
        value = v;
        base = 0x3300;
        if (value < 0)
        {
            u16 tile = base;
            u32 addr = (row << 6) + 0x0600E000;
            u16 *dst = (u16 *)(col + addr);
            s32 j;
            for (j = 0; j < 1; j++)
            {
                *dst = tile;
                tile++;
                dst++;
            }
        }
        else
        {
            u32 addr = (row << 6) + 0x0600E000;
            u16 *dst = (u16 *)(col + addr);
            s32 digit = 1;
            do
            {
                s32 m = Mod(value, 10);
                if (digit != 1 && value == 0)
                    *dst = base + 0xD;
                else
                    *dst = base + m;
                value = Div(value, 10);
                dst--;
                digit++;
            } while (digit <= 3 || value != 0);
        }
    }
}

/* Status screen: draw the current ATK and DEF as right-aligned numbers
 * (clamped to 999; a negative value draws one blank tile). Twin of Menu_DrawCurrentStats. */
void Menu_DrawAttackDefense(s32 x, s32 y)
{
    s16 *stat = &gEwramData->playerState.currentATK;
    s32 i;
    for (i = 0; i <= 1; i++, stat++)
    {
        s32 col = x * 2;
        s32 v = *stat;
        s32 value;
        s32 row;
        u16 base;
        if (v > 1000)
            v = 999;
        row = y + i;
        value = v;
        base = 0x3300;
        if (value < 0)
        {
            u16 tile = base;
            u32 addr = (row << 6) + 0x0600E000;
            u16 *dst = (u16 *)(col + addr);
            s32 j;
            for (j = 0; j < 1; j++)
            {
                *dst = tile;
                tile++;
                dst++;
            }
        }
        else
        {
            u32 addr = (row << 6) + 0x0600E000;
            u16 *dst = (u16 *)(col + addr);
            s32 digit = 1;
            do
            {
                s32 m = Mod(value, 10);
                if (digit != 1 && value == 0)
                    *dst = base + 0xD;
                else
                    *dst = base + m;
                value = Div(value, 10);
                dst--;
                digit++;
            } while (digit <= 3 || value != 0);
        }
    }
}

/* Draw a time value (frames -> seconds, minutes, hours) as three two-digit numbers on the BG map. */
static inline u16 *MapAddr(s32 x, s32 y)
{
    s32 xo = x << 1;
    s32 yo = y << 6;
    return (u16 *)(xo + ({ s32 t_ = yo + BG_MAP_VRAM; t_; }));
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

/**
 * @brief Mark every slot of the six four-entry equipment/soul lists as empty
 * (0xFF) using the existing |= 0xFF stores.
 */
void Player_ClearEquipmentAndSoulLists(void)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    s32 i;

    for (i = 0; i <= 3; i++)
    {
        player->weaponList[i] |= 0xFF;
        player->armorList[i] |= 0xFF;
        player->accessoryList[i] |= 0xFF;
        player->redSoulList[i] |= 0xFF;
        player->blueSoulList[i] |= 0xFF;
        player->yellowSoulList[i] |= 0xFF;
    }
}

/* MenuText_PrintWithInsertedEntry: print a text line with embedded sub-strings (codes 6 = newline, 0x13 = insert, 10 = end); matched. */
void MenuText_PrintWithInsertedEntry(s32 id, s32 arg)
{
    s32 x = gEwramData->text12Column;
    u16 y = gEwramData->text12Row;
    s32 line = 0;
    s32 running = 1;
    u8 *str = Text_GetLocalizedEntry(id);
    u8 *sub = NULL;
    s32 pos = 0;
    s32 subPos = 0;
    u8 *p;
    s32 i;
    s32 c;

    Text12_ClearRegion();
loop:
    {
        if (sub != NULL)
        {
            p = sub;
            i = subPos;
        }
        else
        {
            p = str;
            i = pos;
        }
        { u8 *q = p + 2; c = Text12_PutChar(q[i]); }
        if (c == -1)
            goto next;
        switch (c & 0xFF)
        {
        case 6:
            i++;
            line++;
            {
                u32 t = x << 16;
                u16 ny = y + line;
                Text12_SetCursor(t >> 16, ny);
            }
            break;
        case 0x13:
            sub = Text_GetLocalizedEntry(arg + 0x38B);
            i = 0;
            break;
        case 10:
            if (sub == NULL)
            {
                running = 0;
                break;
            }
            sub = NULL;
            i = pos;
        default:
        next:
            i++;
            break;
        }
        if (sub != NULL)
            subPos = i;
        else
            pos = i;
    }
    if (running)
        goto loop;
    /* the loop doubles the reference count of y so that it gets r8 */
    do /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        Text12_SetCursor(x, y);
    while (0);
}

/* Select paired text entries for the observed button masks.
 * Keep the data/table register lifetimes and short offset binding: agbcc
 * otherwise changes the initial address calculation. Matching bytes do
 * not establish the original source spelling. */
/**
 * @brief Print paired glyphs for the configured action button index and return the selected A/B/L/R mask; retain the default-mask branch.
 */
u16 MenuText_PrintActionButton(s32 action)
{
    register struct EwramData *data asm("r2") = gEwramData; /* FAKEMATCH: pinned register */
    register u16 *buttons asm("r1"); /* FAKEMATCH: pinned register */
    u32 button;
    u16 ret;

    action <<= 1;
    {
        u32 offset = OFFSET_OF(struct EwramData, playerStatus.attackButton);
        buttons = (u16 *)((u8 *)data + offset);
    }
    button = *(u16 *)((u8 *)buttons + action);

    if (button == 1)
    {
        Text12_PutChar(0x80);
        Text12_PutChar(0x81);
        ret = 1;
    }
    else if (button == 2)
    {
        Text12_PutChar(0x82);
        Text12_PutChar(0x83);
        ret = 2;
    }
    else if (button == 0x200)
    {
        Text12_PutChar(0x84);
        Text12_PutChar(0x85);
        ret = button;
    }
    else
    {
        Text12_PutChar(0x86);
        Text12_PutChar(0x87);
        ret = 0x100;
    }
    return ret;
}

/**
 * @brief Print the localized entry with newline, button and paired-symbol control codes, then restore the starting logical column/row through Text12_SetCursor.
 */
void MenuText_Print(s32 index)
{
    u32 x = gEwramData->text12Column;
    u32 y = gEwramData->text12Row;
    s32 lines = 0;
    s32 cont = 1;
    u8 *text = Text_GetLocalizedEntry(index);
    u8 *sub = NULL;
    s32 pos = 0;
    s32 subPos = 0;
    s32 i;
    s32 c;

    do
    {
        u8 *p;
        if (sub)
        {
            p = sub;
            i = subPos;
        }
        else
        {
            p = text;
            i = pos;
        }
        p += 2;
        c = Text12_PutChar(p[i]);
        if (c == -1)
        {
            i++;
        }
        else
        {
            switch (c & 0xFF)
            {
            case 6:
                i++;
                lines++;
                Text12_SetCursor(x, y + lines);
                break;
            case 0xB:
                MenuText_PrintActionButton(0);
                i++;
                break;
            case 0xC:
                MenuText_PrintActionButton(1);
                i++;
                break;
            case 0xD:
                MenuText_PrintActionButton(2);
                i++;
                break;
            case 0xE:
                MenuText_PrintActionButton(3);
                i++;
                break;
            case 0xF:
                gEwramData->text12ColumnPixelOffset++;
                gEwramData->text12PixelX++;
                Text12_PutChar(0x88);
                i++;
                break;
            case 0x10:
                gEwramData->text12ColumnPixelOffset++;
                gEwramData->text12PixelX++;
                Text12_PutChar(0x89);
                i++;
                break;
            case 0x11:
                Text12_PutChar(0x8C);
                Text12_PutChar(0x8D);
                i += 2;
                break;
            case 0x12:
                Text12_PutChar(0x8A);
                Text12_PutChar(0x8B);
                i += 2;
                break;
            case 0xA:
                if (!sub)
                {
                    cont = 0;
                    break;
                }
                sub = NULL;
                i = pos;
                i++;
                break;
            default:
                i++;
                break;
            }
        }
        if (sub)
            subPos = i;
        else
            pos = i;
    } while (cont);

    Text12_SetCursor(x, y);
}

/**
 * @brief 46D68 | Print value in decimal ending at (x, y) of the BG map,
 * right-aligned, at least digits digits, with the digit tiles 0xF300
 * (0xF100 when gameState.subMode == 9). Returns the number of digits written.
 */
s32 HudMap_PrintRightAlignedNumber(s32 x, s32 y, s32 value, s32 digits)
{
    u16 *map;
    s32 attr;
    s32 i;

    if (gEwramData->gameState.subMode == 9)
        attr = 0xF100;
    else
        attr = 0xF300;
    map = BgMapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        *map = Mod(value, 10) + attr;
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

/* 6DD4. */
/* Write count consecutive tile numbers starting at tile into the BG map at
 * VRAM 0x0600E000 (32 tiles per row), from (x, y) rightwards. */
void HudMap_WriteTiles(s32 x, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        tile++;
        map++;
    }
}

/* Clear count consecutive entries of the BG map at VRAM 0x0600E000
 * (32 tiles per row) from (x, y) rightwards. */
void HudMap_ZeroTiles(s32 x, s32 y, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = 0;
        map++;
    }
}

/* Menu helpers 0x08046E28..0x08046FFB: two-line text entries and the
 * decimal number printers. */

/**
 * @brief Set the menu page mode unk_4F9 and clear unk_4FA.
 */
void MenuPage_SetMode(struct EwramData_EntityData *entity, u8 mode)
{
    entity->ext.menuPage.top = 0;
    entity->ext.menuPage.cursor = mode;
}

/**
 * @brief 46E30 | Nothing.
 */
void Menu_EmptyCallback(void)
{
}

/**
 * @brief Print the first segment of localized entry id at the current Text12 cursor,
 * then continue with the following segment at (0, 2).
 */
void Text12_PrintTwoLineEntry(s32 index)
{
    u8 *text = Text_GetLocalizedEntry(index);
    s32 offset = Text12_Print(text, 0);

    Text12_SetCursor(0, 2);
    Text12_Print(text, offset + 1);
}

/**
 * @brief Print value in decimal ending at (x, y) of the BG map with the tiles
 * attr + digit, at least digits places; leading places past the first are
 * blanked with attr + 13 unless zeroPad. A negative value writes the single
 * tile attr. Returns the number of places written.
 */
s32 HudMap_PrintNumberAttr(s32 x, s32 y, s32 value, s32 digits, s32 attr, s32 zeroPad)
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
            *map = d + attr;
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

/**
 * @brief Print value right-aligned at (x, y), zero-padded to digits places, with
 * the fixed digit attributes 0x3300; a negative value writes one tile.
 * Returns the number of places written.
 */
s32 HudMap_PrintPaddedNumber(s32 x, s32 y, s32 value_, s32 digits)
{
    u16 *map;
    s32 i;
    s32 value;
    s32 attr = 0x3300;

    value = value_;

    if (value < 0)
    {
        DrawTileRow(x, y, attr, 1);
        return 1;
    }

    map = BgMapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        *map = Mod(value, 10) + attr;
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

/**
 * @brief As HudMap_PrintPaddedNumber with the digit tiles 0x300 in the given
 * palette.
 */
s32 HudMap_PrintPaddedNumberInPalette(s32 x, s32 y, s32 value_, s32 digits, s32 palette)
{
    u16 *map;
    s32 i;
    s32 value;
    s32 attr = (palette << 12) | 0x300;

    value = value_;

    if (value < 0)
    {
        DrawTileRow(x, y, attr, 1);
        return 1;
    }

    map = BgMapAddr(x, y);
    for (i = 1; i <= digits || value != 0; i++)
    {
        *map = attr + Mod(value, 10);
        value = Div(value, 10);
        map--;
    }
    return i - 1;
}

/* Body of HudMap_PrintNumberAttr, inlined (attr computed by the caller). */
static inline s32 PrintNumber2(s32 x, s32 y, s32 value, s32 digits, s32 attr, s32 zeroPad)
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

/**
 * @brief 46FFC | Like HudMap_PrintNumberAttr with attr = palette << 12 | 0x300
 * (number printer; the shared body is inlined to match the original order).
 */
s32 HudMap_PrintNumber(s32 x, s32 y, s32 value, s32 digits, s32 palette, s32 zeroPad)
{
    return PrintNumber2(x, y, value, digits, palette << 12 | 0x300, zeroPad);
}

/* BG map helpers 0x08047098..0x08047253.
 * All of them write the BG screen map at VRAM 0x0600E000
 * (32 entries per row); 0x330D is the blank tile used by the menus. */

/* Fill count entries with the blank tile from (x, y) leftwards. */
void HudMap_ClearTilesLeft(s32 x, s32 y, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = 0x330D;
        map--;
    }
}

/* Print value in decimal ending at (x, y), right-aligned, using the
 * two-row digit tiles of the given palette; at least digits digits. */
void HudMap_PrintLargeNumber(s32 x, s32 y, s32 value, s32 digits, s32 palette)
{
    u16 *map;
    s32 attr;
    s32 i;

    attr = (palette << 12) | 0x300;
    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 1; i <= digits || value != 0; i++)
    {
        s32 digit = Mod(value, 10);

        map[0] = attr + 0x10 + digit;
        map[0x20] = attr + 0x20 + digit;
        value = Div(value, 10);
        map--;
    }
}

/* Fill count entries with the blank tile from (x, y) rightwards. */
void HudMap_ClearTiles(s32 x, s32 y, s32 count)
{
    u16 tile = 0x330D;
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}

/* Fill the whole 32x32 map with the blank tile. */
void HudMap_ClearAll(void)
{
    DMA_FILL_32(3, 0x330D330D, VRAM_BASE + 0xE000, 0x800);
}

/* Put the tile selected by index from the two ROM tables at (x, y)
 * (written out: the inlined DrawTileRow allocates registers differently). */
void HudMap_DrawStatChangeIndicator(s32 x, s32 y, s32 index)
{
    s32 tiles[3];
    s32 palettes[3];
    u16 tile;
    s32 attr;
    u16 *map;
    s32 i;

    memcpy(tiles, sHudMapTiles, sizeof(tiles));
    memcpy(palettes, sHudMapPalettes, sizeof(palettes));
    attr = (palettes[index] << 12) | 0x300;
    tile = attr + (tiles[index] + 0x4A);
    x <<= 1;
    y <<= 6;
    y += 0x0600E000;
    map = (u16 *)(x + y);

    for (i = 0; i < 1; i++)
    {
        *map = tile;
        tile++;
        map++;
    }
}

/* Draw the 4-tile indicator selected by bits 0..2 of statusEffects at (x, y). */
void HudMap_DrawStatusConditionIcon(s32 x, s32 y)
{
    s32 n = 0;
    s32 flags = gEwramData->playerStatus.statusEffects;

    if (flags & 1)
        n = 2;
    else if (flags & 2)
        n = 3;
    else if (flags & 4)
        n = 1;

    DrawTileRow(x, y, (n << 4) | 0x330A, 4);
}

/* Fill a 16-by-16 block at the top left of the 32-entry-wide BG map.
 * Explicit registers retain agbcc's separate row and tile counters. The
 * inner countdown reuses index; next carries the following outer index. */
void HudMap_DrawTileBlock16(s32 base)
{
    s32 y = 0;
    s32 index = 0;
    s32 x = 0;
    s32 vram = 0x600E000;
    do
    {
        u16 *map = (u16 *)(x + (y * 64 + vram));
        s32 offset = index * 16;
        s32 tile;
        s32 next;
        y++;
        asm("" : "+r"(y)); /* FAKEMATCH: keeps y * 64 (no strength reduction) */
        next = index + 1;
        /* Keep offset separate: agbcc otherwise reverses the adds operands. */
        tile = base + offset;
        index = 15;
        do
        {
            *map = tile;
            map++;
            tile++;
            index--;
        } while (index >= 0);
        index = next;
    } while (index <= 15);
}
