/* Ability soul menu helpers 0x0804BCA0..0x0804BD37. */
#include "code_08032444.h"
#include "menu_page.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* Declared without the u16 tile parameter here: the original passes the
 * tile untruncated (implicit declaration). */
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);

/**
 * @brief 4BCA0 | Show the name of ability soul unk_4F9 when it is owned,
 * otherwise text 0x34F.
 */
void AbilityMenu_PrintSelectedName(struct EwramData_EntityData *entity)
{
    s32 text;

    if (SoulInventory_GetSoulTotal(3, (s8)MENU_PAGE_CURSOR(entity)))
        text = (s8)MENU_PAGE_CURSOR(entity) + 0x257;
    else
        text = 0x34F;

    Text12_ClearRegion();
    MenuText_Print(text);
}

/**
 * @brief 4BCDC | Draw the on/off marker of every owned ability soul
 * (two columns, unk_4F8 entries).
 */
void AbilityMenu_DrawMarkers(struct EwramData_EntityData *entity)
{
    s32 i;

    for (i = 0; i < MENU_PAGE_COUNT(entity); i++)
    {
        if (SoulInventory_GetSoulTotal(3, i))
        {
            s32 x;
            s32 y;

            x = (i & 1) ? 0x1B : 0xD;
            y = (i >> 1) * 2 + 4;
            HudMap_WriteTiles(x, y, ((Player_HasAbility(i) >> i) << 4) + 0xB30E, 2);
        }
    }
}
