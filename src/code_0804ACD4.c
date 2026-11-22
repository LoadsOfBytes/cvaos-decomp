/* Aggregate equipment bonuses, optionally replacing one equipped slot. */
#include <string.h>
#include "structs/ewram.h"
#include "structs/equipment_stats.h"
#include "code/code_080211F0.h"

extern struct WeaponInfo sWeaponInfo[];
extern const struct ArmorInfo sArmorInfo[];

/**
 * @brief Clear the shared preview and aggregate equipped bonuses with an optional category override; exclude weapon flags and preserve unchecked category indexing.
 */
void Equipment_PrepareBonuses(s32 category, s32 item)
{
    struct EwramData *ewram = gEwramData;
    struct EquipmentStats *preview = (struct EquipmentStats *)ewram->sharedScratchBuffer;
    struct PlayerStatus *player = &ewram->playerStatus;
    u8 equipped[3];
    s32 i;
    memset(preview, 0, sizeof(*preview));
    equipped[0] = player->equippedWeapon;
    equipped[1] = player->equippedArmor;
    equipped[2] = player->equippedAccessory;
    if (category >= 0)
        equipped[category] = item;
    for (i = 0; i <= 2; i++)
    {
        if (equipped[i] != 0xFF)
        {
            const struct ArmorInfo *bonus;
            if (i == 0)
                bonus = (const struct ArmorInfo *)&sWeaponInfo[equipped[0]];
            else
                bonus = &sArmorInfo[equipped[i]];
            preview->attack = bonus->attack + preview->attack;
            preview->defense = bonus->defense + preview->defense;
            preview->stats[0] = preview->stats[0] + bonus->stats[0];
            preview->stats[1] = preview->stats[1] + bonus->stats[1];
            preview->stats[2] = preview->stats[2] + bonus->stats[2];
            preview->stats[3] = preview->stats[3] + bonus->stats[3];
            if (i != 0)
                preview->flags |= bonus->flags;
        }
    }
}

static inline void CalculateStats(s32 category, s32 item)
{
    struct EwramData *data = gEwramData;
    struct EquipmentStats *stats = (struct EquipmentStats *)data->sharedScratchBuffer;
    struct PlayerStatus *player = &data->playerStatus;
    Equipment_PrepareBonuses(category, item);
    stats->stats[0] = stats->stats[0] + player->baseStats[0];
    stats->stats[1] = stats->stats[1] + player->baseStats[1];
    stats->stats[2] = stats->stats[2] + player->baseStats[2];
    stats->stats[3] = stats->stats[3] + player->baseStats[3];
}

/**
 * @brief Prepare equipped bonuses plus base stats, copy prepared fields to player state, apply prepared stats and recalculate attack/defense.
 */
void Player_RefreshEquipmentStats(void)
{
    struct EwramData **ewram = &gEwramData;
    struct EwramData *data = *ewram;
    struct EquipmentStats *preview = (struct EquipmentStats *)data->sharedScratchBuffer;
    CalculateStats(-1, 0);
    {
        struct EwramData *current = *ewram;
        current->playerState.preparedStats[0] = preview->stats[0];
        current->playerState.preparedStats[1] = preview->stats[1];
        current->playerState.preparedStats[2] = preview->stats[2];
        current->playerState.preparedStats[3] = preview->stats[3];
        current->playerState.preparedResistances = preview->flags;
        current->playerState.equipmentAttackBonus = preview->attack;
        current->playerState.equipmentDefenseBonus = preview->defense;
    }
    Player_ApplyPreparedStats();
    Player_RecalculateAttackDefense();
}
