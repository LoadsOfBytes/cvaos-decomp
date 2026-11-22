/* Read the inventory count for consumables, weapons or armour. */
#include "structs/ewram.h"
extern const void *Item_GetInfo(s32,s32);
/**
 * @brief Return the owned count of a consumable, weapon or armor (types 2..4).
 */
u32 Inventory_GetOwnedCount(s32 category,s32 index)
{
    struct EwramData *ram=gEwramData;
    u32 result=(u32)Item_GetInfo(category,index);
    switch(category)
    {
    case 2:return ram->playerStatus.itemInventory[index];
    case 3:return ram->playerStatus.weaponInventory[index];
    case 4:return ram->playerStatus.armorInventory[index];
    }
    return result;
}
