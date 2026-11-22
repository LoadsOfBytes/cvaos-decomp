/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyBomberArmorUpdate = EnemyAxeArmorUpdate */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void BomberRockArmor_UpdateWalk(struct EwramData_EntityData *entity);
extern void BomberArmor_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 BomberRockArmor_UpdateDeath(struct EwramData_EntityData *entity);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);

/**
 * @brief B3040 | Same as EnemyAxeArmorUpdate.
 */
void EnemyBomberArmorUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            BomberRockArmor_UpdateWalk(entity);
            break;
        case 1:
            BomberArmor_UpdateAttack(entity);
            break;
        case 2:
            if (BomberRockArmor_UpdateDeath(entity))
            {
                EntityDelete(entity);
                return;
            }
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}
