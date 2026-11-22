/* Item tables 0x08044E94..0x08044EDB (Item_GetInfoByGlobalId is in code_08044EDC.c, the rest
 * of the item helpers are in code_08044F24.c).
 *
 * Global item ids are 1-based: 1..0x20 usable items (type 2), 0x21..0x5B
 * weapons (type 3), 0x5C..0x88 armor and accessories (type 4). */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/equipment_stats.h"

#define GOLD_MAX 999999

/* Usable item info (16-byte records). */
struct ItemInfo {
    u8 pad_0[0x10];
};

extern const struct ItemInfo sConsumableInfo[];
extern struct WeaponInfo sWeaponInfo[]; // weapons, see code_080211F0.c
extern const struct ArmorInfo sArmorInfo[];

/**
 * @brief Return the indexed record for item, weapon or armor type 2..4, otherwise NULL; preserve unchecked per-type indices.
 */
const void *Item_GetInfo(s32 category, s32 index)
{
    const void *info = NULL;

    switch (category)
    {
        case 2:
            info = &sConsumableInfo[index];
            break;
        case 3:
            info = &sWeaponInfo[index];
            break;
        case 4:
            info = &sArmorInfo[index];
            break;
    }
    return info;
}
