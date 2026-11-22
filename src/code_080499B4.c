#include "gba.h"
#include "structs/ewram.h"
#include "code_08032444.h"

struct SoulMenu
{
    u8 pad[0x13];
    u8 type;
    u8 count;
    s8 cursor;
    u8 pad16;
    u8 selected;
};

extern void Player_RefreshEquipmentStats(void);
extern void YellowSoul_Stop(void);
extern void SoulList_UpdateForEquip(struct SoulMenu *menu);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void Menu_DrawAttackDefense(s32 a, s32 b);
extern void Menu_DrawCurrentStats(s32 a, s32 b);
extern void HudMap_DrawStatusConditionIcon(s32 a, s32 b);
extern void SoulMenu_ClearStatPreview(void);

/**
 * @brief Equip the selected soul (0 unequips) in the menu's red/blue/yellow slot:
 * refresh stats, stop the yellow soul when a soul was equipped, update the
 * soul list, restart the yellow soul, adjust the list count/cursor when the
 * last copy was equipped and redraw the stats.
 */
void SoulMenu_EquipSelected(struct SoulMenu *menu)
{
    u8 *equip;
    u8 *slot;
    s32 type;
    u8 sel;
    u8 count;

    equip = (u8 *)gEwramData + 0x1325C;
    switch (menu->type)
    {
        case 0:
            slot = (u8 *)gEwramData + 0x13269;
            type = 0;
            break;
        case 1:
            slot = (u8 *)gEwramData + 0x1326A;
            type = 1;
            break;
        case 2:
            slot = (u8 *)gEwramData + 0x1326B;
            type = 2;
            break;
    }
    Player_RefreshEquipmentStats();
    if (*slot != 0)
    {
        if (SoulInventory_GetSoulTotal(type, *slot - 1) == 1)
            menu->count++;
        YellowSoul_Stop();
    }
    SoulList_UpdateForEquip(menu);
    if (menu->selected != 0)
    {
        *slot = menu->selected;
        if (type == 2)
            YellowSoul_Restart(menu->selected, 1);
        else
            YellowSoul_Restart(equip[0xF], 1);
        if (SoulInventory_GetSoulTotal(type, menu->selected - 1) == 0)
        {
            count = menu->count;
            menu->count = count - 1;
            if (menu->cursor > (s8)menu->count - 1)
                menu->cursor = count - 2;
        }
    }
    else
    {
        *slot = 0;
        if (type != 2)
            YellowSoul_Restart(equip[0xF], 1);
    }
    Menu_DrawAttackDefense(0x16, 2);
    Menu_DrawCurrentStats(0x16, 4);
    HudMap_DrawStatusConditionIcon(0x15, 8);
    SoulMenu_ClearStatPreview();
}
