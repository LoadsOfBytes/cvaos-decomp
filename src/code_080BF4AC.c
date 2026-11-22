/* Death_OnHit (Death). */
#include "structs/ewram.h"
#include "death_entity.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void BossDefeatEffect_Spawn(s32 x, s32 y);

/**
 * @brief Mark the boss hit flag, then apply enemy damage. On defeat drop
 * rewards, spawn the defeat effect above the boss, restore its palette
 * and enter death state 7 with substate counters reset.
 */
void Death_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    DEATH_FLAGS(entity) |= 2;
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 7;
        entity->stage = 0;
        entity->subState = 0;
    }
}
