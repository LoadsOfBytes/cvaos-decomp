/* DiscArmor_OnHit. */
#include "structs/ewram.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *param_0, s32 param_1, u16 *param_2);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief B2370 | Hit: Entity_FaceTarget in state 0; when killed (Enemy_TakeDamage),
 * Enemy_DropRewards, palette into 0x53E bits 3-6 and state 2.
 */
void DiscArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (entity->state == 0)
        Entity_FaceTarget(entity, attacker, hit);

    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}
