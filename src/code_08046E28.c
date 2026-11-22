/* Menu helpers 0x08046E28..0x08046FFB: two-line text entries and the
 * decimal number printers. */
#include "code/code_08039340.h"
#include "menu_page.h"
#include "code/code_08040A38.h"
#include "bg_map.h"
#include "structs/ewram.h"
#include "syscalls.h"

/**
 * @brief Set the menu page mode unk_4F9 and clear unk_4FA.
 */
void MenuPage_SetMode(struct EwramData_EntityData *entity, u8 mode)
{
    MENU_PAGE_TOP(entity) = 0;
    MENU_PAGE_CURSOR(entity) = mode;
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
