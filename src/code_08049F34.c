#include "gba.h"
#include "structs/ewram.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"
#include "structs/equipment_stats.h"

extern struct WeaponInfo sWeaponInfo[];
extern const struct ArmorInfo sArmorInfo[];
extern const u16 sItemNameTextIds[];

/**
 * @brief Print the equipped weapon, armor and accessory names (or the none text
 * 0x34F) into their one-row Text8 regions of the equipment menu.
 */
void EquipmentMenu_PrintEquippedNames(void)
{
    struct EwramData *ew;
    s32 text;
    s32 i;
    s32 j;
    s32 odd;
    u8 *items;
    u8 item;
    struct PlayerStatus *equip;

    ew = gEwramData;
    equip = &ew->playerStatus;
    text = 0x34F;
    if (equip->equippedWeapon != 0xFF)
    {
        struct WeaponInfo *w = &((struct WeaponInfo *)0x08505D3C)[equip->equippedWeapon];
        text = sItemNameTextIds[w->name];
    }
    Text8_SetRegion(0x15, 8, 0xE, 1);
    Text8_ClearRegion();
    Text8_SetPixelCursor(0x80, 8);
    Text8_Print(Text_GetLocalizedEntry(text), 0);
    i = 1;
    items = &ew->playerStatus.equippedArmor;
loop:
    {
        text = 0x34F;
        j = i - 1;
        if (items[j] != 0xFF)
        {
            const struct ArmorInfo *a = &((const struct ArmorInfo *)0x085063B0)[items[j]];
            text = sItemNameTextIds[a->name];
        }
        odd = (i + 1) & 1;
        Text8_SetRegion(odd ? 0x15 : 0, ((i + 1) >> 1) + 8, 0xE, 1);
        Text8_ClearRegion();
        if (odd)
            Text8_SetPixelCursor(0x80, 9);
        Text8_Print(Text_GetLocalizedEntry(text), 0);
    }
    if (++i <= 2)
        goto loop;
}
