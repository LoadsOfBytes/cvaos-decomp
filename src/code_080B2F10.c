/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BomberRockArmor_OnHit = AxeArmor_OnHit */
#include "structs/ewram.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief B2F10 | Same as AxeArmor_OnHit.
 */
void BomberRockArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Entity_FaceTarget(entity, attacker, hit);
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->palette = entity->basePalette;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief B2F5C | Clear the HP.
 */
void ArmorProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->hp = 0;
}
