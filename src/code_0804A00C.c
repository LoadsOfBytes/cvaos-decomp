/* EquipmentMenu_ShowSlotDescription; slot switch as in
 * EquipmentMenu_DrawEquippedIcons. */
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_080211F0.h"
#include "structs/equipment_stats.h"

extern struct WeaponInfo sWeaponInfo[];
extern const struct ArmorInfo sArmorInfo[];
extern const u16 sItemDescriptionTextIds[];
extern void MenuItemIcon_Show(s32 x, s32 y, s32 type, s32 item);
extern void MenuItemIcon_SetHidden(u8 value);

/* Same caller-owned record as in code_0804A0B4.c. */
struct EquipmentMenuItemsView {
    u8 pad_0[0xC];
    u8 equippedIds[6];
};

/* Cursor record of the caller (asm in code_08049F34.s); slot at 0xB. */
struct EquipmentSlotCursorView {
    u8 pad_0[0xB];
    u8 slot;
};

/**
 * @brief Show the description and icon of the equipment slot under the cursor
 * (unk_B): slot 0 shows weapon unk_C[0] (type 3), slots 1-2 the
 * armor/accessory unk_C[slot + 3] (type 4), each named through
 * sItemDescriptionTextIds; an empty slot (0xFF) shows text 0x351 and hides the icon
 * (MenuItemIcon_SetHidden(1)).
 */
void EquipmentMenu_ShowSlotDescription(struct EquipmentMenuItemsView *stats, struct EquipmentSlotCursorView *page)
{
    s32 text;
    s32 item;
    s32 type;
    s32 slot;

    Text12_ClearRegion();
    text = 0x351;
    item = -1;
    slot = page->slot;
    switch (slot)
    {
    case 0:
        if (stats->equippedIds[0] != 0xFF)
        {
            s32 id = stats->equippedIds[0];
            struct WeaponInfo *info = &sWeaponInfo[id];
            text = sItemDescriptionTextIds[info->name];
            type = 3;
            item = id;
        }
        break;
    case 1:
    case 2:
        if (stats->equippedIds[slot + 3] != 0xFF)
        {
            s32 id = stats->equippedIds[slot + 3];
            const struct ArmorInfo *info = &sArmorInfo[id];
            text = sItemDescriptionTextIds[info->name];
            type = 4;
            item = id;
        }
        break;
    }
    MenuText_ClearAndPrint(text);
    if (item != -1)
    {
        MenuItemIcon_Show(0x1C, 0x94, type, item);
        MenuItemIcon_SetHidden(0);
    }
    else
        MenuItemIcon_SetHidden(1);
}
