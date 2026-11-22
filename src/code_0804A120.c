#include "gba.h"
#include "structs/ewram.h"

struct MenuPage
{
    u8 pad_0[0xB];
    u8 subState;
    u8 pad_C[0x17 - 0xC];
    u8 unk_17;
};

extern void Player_RefreshEquipmentStats(void);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void HudMap_DrawStatChangeIndicator(s32 a, s32 b, s32 c);
extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

/**
 * @brief Temporarily equip the highlighted item, refresh stats/yellow soul and draw stat values and comparison indicators, then restore equipment and recalculate.
 */
void EquipmentMenu_DrawStatPreview(struct MenuPage *page)
{
    s16 *stats;
    s16 old[6];
    u8 saved[2];
    u8 savedWeapon;
    u8 *weapon;
    u8 *armor;
    u8 *accessory;
    s32 i;
    s16 kind;
    s32 color;
    s32 value;
    s16 diff;
    s32 row;

    stats = &gEwramData->playerState.currentATK;
    for (i = 0; i < 6; i++)
        old[i] = stats[i];
    weapon = &gEwramData->playerStatus.equippedWeapon;
    savedWeapon = *weapon;
    armor = &gEwramData->playerStatus.equippedArmor;
    saved[0] = *armor;
    accessory = &gEwramData->playerStatus.equippedAccessory;
    saved[1] = *accessory;
    switch (page->subState)
    {
    case 0:
        *weapon = page->unk_17;
        break;
    case 1:
        *armor = page->unk_17;
        break;
    case 2:
        *accessory = page->unk_17;
        break;
    }
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
    for (i = 0; i < 6; i++)
    {
        diff = stats[i] - old[i];
        if (diff == 0)
        {
            kind = 0;
            color = 3;
        }
        else if (diff > 0)
        {
            kind = 1;
            color = 9;
        }
        else
        {
            kind = 2;
            color = 8;
        }
        value = stats[i];
        if (value > 1000)
            value = 999;
        if (i <= 1)
        {
            row = i + 2;
            HudMap_DrawStatChangeIndicator(0x18, row, kind);
            HudMap_PrintNumber(0x1B, row, value, 3, color, 0);
        }
        else
        {
            row = i + 3;
            HudMap_DrawStatChangeIndicator(0x18, row, kind);
            HudMap_PrintNumber(0x1B, row, value, 3, color, 0);
        }
    }
    gEwramData->playerStatus.equippedWeapon = savedWeapon;
    gEwramData->playerStatus.equippedArmor = saved[0];
    gEwramData->playerStatus.equippedAccessory = saved[1];
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
}
