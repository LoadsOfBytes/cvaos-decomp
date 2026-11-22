/* Menu helpers 0x08047330..0x080473F3. */
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "bg_map.h"

/**
 * @brief Clear the 0x1E x 2 Text12 region and print the name of soul (index + 1)
 * of the given kind, or the default text 0x34F when soul is zero. Kinds
 * outside 0..3 keep the default base.
 */
void Menu_PrintSoulName(s32 soulType, s32 soulId)
{
    s32 text = 0x34F;

    if (soulId != 0)
    {
        switch (soulType)
        {
            case 0:
                text = 0x1E4;
                break;
            case 1:
                text = 0x21C;
                break;
            case 2:
                text = 0x234;
                break;
            case 3:
                text = 0x257;
                break;
        }
        text += soulId - 1;
    }

    Text12_SetRegion(0, 0, 0x1E, 2);
    Text12_ClearRegion();
    MenuText_Print(text);
}

/**
 * @brief Print the first segment of localized entry id at (0, row) and the
 * following segment at (0, row + 1).
 */
void Text12_PrintTwoLineEntryAt(s32 index, s32 row)
{
    u8 *text = Text_GetLocalizedEntry(index);
    s32 offset;

    Text12_SetCursor(0, row);
    offset = Text12_Print(text, 0);
    Text12_SetCursor(0, row + 1);
    Text12_Print(text, offset + 1);
}

/**
 * @brief Fill count entries of the BG map with the same tile from (x, y) rightwards.
 */
void HudMap_FillTiles(s32 x, s32 y, u16 tile, s32 count)
{
    u16 *map;
    s32 i;

    x <<= 1;
    y <<= 6;
    y += BG_MAP_VRAM;
    map = (u16 *)(x + y);

    for (i = 0; i < count; i++)
    {
        *map = tile;
        map++;
    }
}
