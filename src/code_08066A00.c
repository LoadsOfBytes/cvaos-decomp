#include "gba.h"
#include "structs/ewram.h"

struct ArmorSlotInfoView {
    u8 pad_0[8];
    u8 armorSlotKind;
    u8 pad_9[0x14 - 9];
};
extern const struct ArmorSlotInfoView sArmorInfo[];

extern void Player_RefreshEquipmentStats(void);
extern void YellowSoul_Restart(u8 soul, s32 force);
extern void HudMap_DrawStatChangeIndicator(s32 a, s32 b, s32 c);
extern void HudMap_PrintNumber(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

/**
 * @brief Preview the stat changes of equipping weapon/armor/accessory id,
 * then restore the equipment and recompute the stats.
 */
void Shop_PreviewEquipStats(void *unused, s32 type, s32 id)
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

    stats = &gEwramData->playerState.currentATK;
    for (i = 0; i < 6; i++)
        old[i] = stats[i];
    weapon = &gEwramData->playerStatus.equippedWeapon;
    savedWeapon = *weapon;
    armor = &gEwramData->playerStatus.equippedArmor;
    saved[0] = *armor;
    accessory = &gEwramData->playerStatus.equippedAccessory;
    saved[1] = *accessory;
    switch (type)
    {
    case 3:
        *weapon = id;
        break;
    case 4:
        if (sArmorInfo[id].armorSlotKind == 0)
            *armor = id;
        else
            *accessory = id;
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
        HudMap_DrawStatChangeIndicator(0x18, i + 2, kind);
        HudMap_PrintNumber(0x1B, i + 2, value, 3, color, 0);
    }
    gEwramData->playerStatus.equippedWeapon = savedWeapon;
    gEwramData->playerStatus.equippedArmor = saved[0];
    gEwramData->playerStatus.equippedAccessory = saved[1];
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
}
