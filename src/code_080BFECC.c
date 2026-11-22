/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EvilButcher_OnHit = HeadStackSegment_OnHit */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *param_0);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 2 with substate counters reset.
 */
void EvilButcher_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
