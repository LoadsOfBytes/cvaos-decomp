/* EnemySkeletonKnightUpdate. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void SkeletonKnight_UpdateWalk(struct EwramData_EntityData *entity);
extern void SkeletonKnight_UpdateAttack(struct EwramData_EntityData *entity);

/**
 * @brief 8F198 | Skeleton Knight update: unless Enemy_PreUpdate handles the
 * frame, run the state (0 SkeletonKnight_UpdateWalk, 1 SkeletonKnight_UpdateAttack, 2 Enemy_UpdateFragmentDeath,
 * 3 only animate) and then Entity_AnimateWithBoxes / Enemy_CheckHitPlayer.
 */
void EnemySkeletonKnightUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            SkeletonKnight_UpdateWalk(entity);
            break;
        case 1:
            SkeletonKnight_UpdateAttack(entity);
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
