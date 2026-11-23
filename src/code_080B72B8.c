/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Chronomage_OnHit = SkullMillioneFamily_OnHit */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, restore the palette
 * and enter death state 3 with the substate counters reset.
 */
void Chronomage_OnHit(struct EwramData_EntityData *entity)
{
    if (Enemy_TakeDamage(entity))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
}
