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
    u8 pad_2[0xA - 0x2];
    u8 attack;
    u8 defense;
    s8 stats[4];
    u16 flags;
    u8 pad_12[2];
};

#endif
