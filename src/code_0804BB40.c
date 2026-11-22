#include "code_08009A0.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"

struct MenuPage
{
    u8 pad_0[0x14];
    s8 count;
    s8 cursor;
};

extern s32 MenuGrid_MoveCursor(struct MenuPage *page, s32 repeated);
extern void MenuPageTitle_Delete(struct EwramData_EntityData *ent);
extern void MenuPageTitle_Update(struct EwramData_EntityData *ent);
extern void HudMap_WriteTiles(s32 x, s32 y, s32 tile, s32 count);
extern void Cursor_SetTilePosition(s32 a, s32 b);
extern void Cursor_Draw(void);

/**
 * @brief 4BB40 | Ability soul page update: A toggles the selected ability
 * (when owned), cursor movement shows its name, B leaves; redraws the
 * on/off markers of all abilities.
 *
 * @return -1 when the page is left, 0 otherwise
 */
s32 AbilityMenu_Update(struct MenuPage *page)
{
    s32 repeated;
    s32 new;
    s32 moved;
    s32 text;
    s32 i;
    s32 x;
    s32 y;

    repeated = gEwramData->inputData.repeatedInput;
    new = gEwramData->inputData.newInput;
    if (new & 1)
    {
        if (SoulInventory_GetSoulTotal(3, page->cursor))
        {
            PlaySong(0xF4);
            if (Player_HasAbility(page->cursor))
                Player_SetAbility(page->cursor, 0);
            else
                Player_SetAbility(page->cursor, 1);
        }
        else
        {
            PlaySong(0xEF);
        }
    }
    else
    {
        moved = MenuGrid_MoveCursor(page, repeated);
        if (new & 2)
        {
            PlaySong(0xF0);
            MenuPageTitle_Delete(&gEwramData->entityData[20]);
        }
        if (moved)
        {
            PlaySong(0xF1);
            if (SoulInventory_GetSoulTotal(3, page->cursor))
                text = page->cursor + 0x257;
            else
                text = 0x34F;
            Text12_ClearRegion();
            MenuText_Print(text);
        }
    }
    Cursor_SetTilePosition((page->cursor & 1) ? 0x10 : 2, (page->cursor >> 1) * 2 + 4);
    Cursor_Draw();
    MenuPageTitle_Update(&gEwramData->entityData[20]);
    for (i = 0; i < page->count; i++)
    {
        if (SoulInventory_GetSoulTotal(3, i))
        {
            x = (i & 1) ? 0x1B : 0xD;
            y = (i >> 1) * 2 + 4;
            HudMap_WriteTiles(x, y, ((Player_HasAbility(i) >> i) << 4) + 0xB30E, 2);
        }
    }
    return (new & 2) ? -1 : 0;
}
