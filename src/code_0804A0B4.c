/* EquipmentMenu_DrawEquippedIcons. */
#include "types.h"

void MenuSlotIcon_Show(s32 x, s32 y, s32 type, s32 item, s32 slot);
void MenuSlotIcon_SetHidden(u8 value, s32 index);

/* Caller-owned record (asm caller in code_08049F34.s); only the item ids at
 * 0xC (slot 0) and 0x10..0x11 (slots 1-2, 0xFF = empty) are read here. */
struct EquipmentMenuItemsView {
    u8 pad_0[0xC];
    u8 equippedIds[6];
};

/**
 * @brief For the three slots at y = 0x32 + 0x10 * slot draw the equipped item
 * (type 3 for slot 0, type 4 otherwise) via MenuSlotIcon_Show and clear the
 * slot's flag with MenuSlotIcon_SetHidden, or set the flag when the slot is empty.
 */
void EquipmentMenu_DrawEquippedIcons(struct EquipmentMenuItemsView *stats)
{
    s32 i;
    s32 y;
    s32 type;
    s32 item;

    i = 0;
    y = 0x32;
    for (; i <= 2; y += 0x10, i++)
    {
        item = -1;
        switch (i)
        {
        case 0:
            if (stats->equippedIds[0] != 0xFF)
            {
                type = 3;
                item = stats->equippedIds[0];
            }
            break;
        case 1:
        case 2:
            if (stats->equippedIds[i + 3] != 0xFF)
            {
                type = 4;
                item = stats->equippedIds[i + 3];
            }
            break;
        }
        if (item != -1)
        {
            MenuSlotIcon_Show(0x70, y, type, item, i);
            MenuSlotIcon_SetHidden(0, i);
        }
        else
            MenuSlotIcon_SetHidden(1, i);
    }
}
