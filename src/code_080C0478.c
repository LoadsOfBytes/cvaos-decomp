/* KickerSkeleton_OnHit: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "structs/ewram.h"

extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Apply enemy damage; on defeat drop rewards, store the attacker-
 * relative fragment direction, restore the palette and enter death state 2.
 */
void KickerSkeleton_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, (s32)attacker, hit))
    {
        Enemy_DropRewards(entity);
        {
            s32 speed = 1;

            entity->ext.kickerSkeleton.hitSide = speed;
            if (entity->posX.whole < attacker->posX.whole)
                entity->ext.kickerSkeleton.hitSide = -speed;
        }
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
