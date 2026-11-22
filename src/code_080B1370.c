/* AxeArmor_OnHit. */
#include "structs/ewram.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *param_0, s32 param_1, u16 *param_2);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief Face the target and apply damage; on death drop rewards, restore the palette and enter state 2.
 */
void AxeArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData)
{
    Entity_FaceTarget(entity, attacker, hitData);
    if (Enemy_TakeDamage(entity, attacker, hitData))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Clear axe health so its next update starts destruction.
 */
void AxeArmorAxe_ClearHealth(struct EwramData_EntityData *entity)
{
    entity->unk_518.part16.hp = 0;
}
