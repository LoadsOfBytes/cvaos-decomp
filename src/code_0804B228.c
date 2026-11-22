/* Equipment stat preview and selection helpers. */
#include "structs/ewram.h"
#include "code/code_08040A38.h"
#include "structs/equipment_stats.h"

extern void Equipment_PrepareBonuses(s32, s32);
extern void YellowSoul_Restart(u8, s32);

/**
 * @brief Prepare equipment bonuses using the supplied override and add base stats into the shared preview; do not apply it to current player stats.
 */
void Equipment_PrepareStatPreview(s32 category, s32 item)
{
    struct EwramData *ewram = gEwramData;
    struct EquipmentStats *preview = (struct EquipmentStats *)ewram->sharedScratchBuffer;
    struct PlayerStatus *player = &ewram->playerStatus;
    Equipment_PrepareBonuses(category, item);
    preview->stats[0] = preview->stats[0] + player->baseStats[0];
    preview->stats[1] = preview->stats[1] + player->baseStats[1];
    preview->stats[2] = preview->stats[2] + player->baseStats[2];
    preview->stats[3] = preview->stats[3] + player->baseStats[3];
}

/**
 * @brief Store the item in the selected equipment category, refresh equipment stats and restart the equipped yellow soul; retain unsupported-category behavior.
 */
void Player_EquipItem(s32 category, s32 item)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    switch (category)
    {
        case 2:
            player->equippedWeapon = item;
            break;
        case 0:
            player->equippedArmor = item;
            break;
        case 1:
            player->equippedAccessory = item;
            break;
    }
    Player_RefreshEquipmentStats();
    YellowSoul_Restart(gEwramData->playerStatus.equippedYellowSoul, 1);
}

/* Store value into the equipment list of the given kind (0 armor,
 * 1 accessory, 2 weapon): only slot 0 when value is nonzero, otherwise all
 * four slots. Same shape as SoulList_WriteValue for the soul lists. */
/**
 * @brief Write the value to slot zero, returning if its low byte is nonzero; otherwise write all four entries and retain the final slot-three assignment.
 */
void EquipmentList_WriteValue(s32 kind, s32 value)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 *list;
    s32 i;

    switch (kind)
    {
        case 2:
            list = player->weaponList;
            break;
        case 0:
            list = player->armorList;
            break;
        case 1:
            list = player->accessoryList;
            break;
    }
    for (i = 0; i <= 3; i++)
    {
        list[i] = value;
        if ((u8)value != 0)
            return;
    }
    list[3] = value;
}

/* Equipped item of the given kind (0 armor, 1 accessory, 2 weapon). */
/**
 * @brief Return the equipped armor, accessory or weapon for categories zero, one or two; retain the unset result for unsupported categories.
 */
u8 Player_GetEquippedItem(s32 kind)
{
    struct PlayerStatus *player = &gEwramData->playerStatus;
    u8 item;

    switch (kind)
    {
        case 2:
            item = player->equippedWeapon;
            break;
        case 0:
            item = player->equippedArmor;
            break;
        case 1:
            item = player->equippedAccessory;
            break;
    }
    return item;
}
