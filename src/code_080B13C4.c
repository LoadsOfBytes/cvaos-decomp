/* EnemyAxeArmorUpdate. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void AxeArmor_UpdateWalking(struct EwramData_EntityData *entity);
extern void AxeArmor_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 AxeArmor_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief B13C4 | Axe Armor update: unless Enemy_PreUpdate handles the frame,
 * run the state (0 AxeArmor_UpdateWalking, 1 AxeArmor_UpdateAttack, 2 AxeArmor_UpdateDeath - delete
 * when it returns nonzero, 3 only animate) and then Enemy_CheckHitPlayer /
 * Entity_AnimateWithBoxes.
 */
void EnemyAxeArmorUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            AxeArmor_UpdateWalking(entity);
            break;
        case 1:
            AxeArmor_UpdateAttack(entity);
            break;
        case 2:
            if (AxeArmor_UpdateDeath(entity))
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
