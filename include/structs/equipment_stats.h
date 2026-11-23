#ifndef EQUIPMENT_STATS_H
#define EQUIPMENT_STATS_H

#include "types.h"

/* Aggregate in the shared buffer at 0x133F4. */
struct EquipmentStats {
    u16 attack;
    u16 defense;
    u16 stats[4];
    u16 flags;
    u16 pad;
};

/* 0x14-byte armor/accessory record; weapons share the bonus prefix. */
struct ArmorInfo {
    u16 name; // index into sItemDescriptionTextIds (EquipmentMenu_ShowSlotDescription)
    u8 pad_2[0x8 - 0x2];
    u8 type; // equipment slot kind (armor / accessory), compared by the equipment lists
    u8 pad_9;
    u8 attack;
    u8 defense;
    s8 stats[4];
    u16 flags;
    u8 pad_12[2];
};

/* 0x1C-byte weapon record (sWeaponInfo); the armor bonus fields (attack ..
 * flags at 0xA..0x11) are read through struct ArmorInfo. */
struct WeaponInfo {
    u16 name; // index into sItemDescriptionTextIds (EquipmentMenu_ShowSlotDescription)
    u8 pad_2[0x8 - 0x2];
    u8 attackAnimKind;
    u8 pad_9[0x10 - 0x9];
    u16 attackAttributes;
    u8 graphicsIndex;
    u8 spriteSheetIndex;
    u8 animScriptIndex;
    u8 paletteIndex;
    u8 attackVariant;
    u8 hitTimer;
    u16 attackSound;
    u8 pad_1A[0x1C - 0x1A];
};

/* 0x10-byte usable item record (sConsumableInfo, Item_GetInfo type 2). */
struct ConsumableInfo {
    u16 name; // index into sItemNameTextIds
    u8 oneBasedIconId;
    u8 pad_3[0x8 - 0x3];
    u8 effect; // ItemMenu_UseItem: 1 HP, 2 MP, 3 status cure, ...
    u8 pad_9;
    u16 value;
    u8 pad_C[0x10 - 0xC];
};

#endif
