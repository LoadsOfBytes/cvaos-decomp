/* DeadWarrior_OnHit: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *param_0);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 1 with the substate counters reset.
 */
void DeadWarrior_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
}
