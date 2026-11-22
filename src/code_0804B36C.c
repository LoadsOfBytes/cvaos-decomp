#include "code/code_080211F0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

struct ConsumableInfo {
    u8 prefix[8];
    u8 effect;
    u8 pad_9;
    u16 value;
    u8 suffix[4];
};
extern const struct ConsumableInfo sConsumableInfo[];
static inline s32 Add16(s16 a, u16 b) { return a + b; }

/* Apply selected consumable: HP/MP recovery, status cure or HP cost. */
s32 ItemMenu_UseItem(struct PlayerStatus *stats, struct EwramData_EntityData *menu)
{
    s32 changed = 0;
    const struct ConsumableInfo *info;
    s32 effect;
    u16 amount, maxMP;
    if ((gEwramData->gameState.gameFlagsTail[0] & 0x100000) || (u8)menu->unk_4F8.part8.unk_4FB == 0xFF)
        return 0;
    info = &sConsumableInfo[(u8)menu->unk_4F8.part8.unk_4FB];
    effect = info->effect;
    if (effect == 3 && Player_TestSoulStateFlags(0x100000)) effect = 0;
    switch (effect)
    {
    case 0:
        if (stats->currentHP < stats->maxHP)
        {
            changed = 1;
            stats->currentHP += info->value;
            if (stats->currentHP > stats->maxHP) stats->currentHP = stats->maxHP;
        }
        break;
    case 1:
        maxMP = stats->maxMP;
        amount = maxMP / (100 / info->value);
        if (stats->currentMP < maxMP)
        {
            changed = 1;
            stats->currentMP = Add16(amount, stats->currentMP);
            if (stats->currentMP > maxMP) stats->currentMP = maxMP;
        }
        break;
    case 2:
        if (info->value & gEwramData->playerStatus.statusEffects)
        {
            changed = 1;
            Player_ClearStatusEffects(info->value);
        }
        break;
    case 3:
        if (stats->currentHP > 1)
        {
            changed = 1;
            stats->currentHP -= info->value;
            if (stats->currentHP <= 0) stats->currentHP = 1;
        }
        break;
    }
    if (effect == 3)
    {
        PlaySong(0x198);
        PlaySong(0xEF);
    }
    else PlaySong(0xF8);
    return changed;
}
