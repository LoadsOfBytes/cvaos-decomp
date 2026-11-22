/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyCurlyUpdate = EnemySkeletonKnightUpdate */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Curly_UpdateWalk(struct EwramData_EntityData *entity);
extern void Curly_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);

/**
 * @brief 936A0 | Same as EnemySkeletonKnightUpdate.
 */
void EnemyCurlyUpdate(struct EwramData_EntityData *entity)
{
    u32 var_0[4];

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            Curly_UpdateWalk(entity);
            break;
        case 1:
            Curly_UpdateAttack(entity);
            break;
        case 2:
            Enemy_UpdateFragmentDeath(entity);
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
