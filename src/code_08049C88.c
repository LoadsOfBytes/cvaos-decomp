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

extern void Player_RefreshEquipmentStats(void);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void HudMap_DrawStatChangeIndicator(s32 a, s32 b, s32 c);
extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

/**
 * @brief Temporarily clear the selected equipped-soul slot, recalculate with the selected yellow soul when appropriate, draw comparison values and restore the saved stats and slot.
 */
void SoulMenu_DrawStatPreview(struct SoulMenu *menu)
{
    s16 *stats;
    u8 *slot;
    u8 saved;
    s16 old[6];
    s32 i;
    s16 kind;
    s32 color;
    s32 value;
    s16 diff;

    stats = (s16 *)((u8 *)gEwramData + 0x131EE);
    slot = (u8 *)gEwramData + 0x13269;
    slot += menu->type;
    saved = *slot;
    *slot = 0;
    for (i = 0; i < 6; i++)
        old[i] = stats[i];
    Player_RefreshEquipmentStats();
    if (menu->type == 2)
        YellowSoul_Restart(menu->selected, 1);
    else
        YellowSoul_Restart(*((u8 *)gEwramData + 0x1326B), 1);
    for (i = 0; i < 6; i++)
    {
        diff = stats[i] - old[i];
        if (diff == 0)
        {
            kind = 0; color = 3;
        }
        else if (diff > 0)
        {
            kind = 1; color = 9;
        }
        else
        {
            kind = 2; color = 8;
        }
        value = stats[i];
        if (value > 1000)
            value = 999;
        HudMap_DrawStatChangeIndicator(0x18, i + 2, kind);
        HudMap_PrintNumber(0x1B, i + 2, value, 3, color, 0);
    }
    for (i = 0; i < 6; i++)
        stats[i] = old[i];
    *slot = saved;
}
