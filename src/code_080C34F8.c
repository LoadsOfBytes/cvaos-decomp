/* EnemySkeletonUpdate. */
/* Skeleton update dispatch and affine rotation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Skeleton_UpdateState5Animation(struct EwramData_EntityData *);
extern void Skeleton_UpdateDeath(struct EwramData_EntityData *);
extern void Skeleton_UpdatePatrol(struct EwramData_EntityData *);
extern void Skeleton_UpdateCombatMovement(struct EwramData_EntityData *);
extern void Skeleton_UpdateIdle(struct EwramData_EntityData *);
extern void Skeleton_UpdateAttackPreparation(struct EwramData_EntityData *);
extern void Skeleton_UpdateThrow(struct EwramData_EntityData *);

void EnemySkeletonUpdate(struct EwramData_EntityData *entity)
{
    { s32 step;
    { register s32 delta asm("r1"); /* FAKEMATCH: pinned register */
    if (!entity->flipX)
        delta = -0x708;
    else
        delta = 0x708;
    step = delta; }
    { register u32 old asm("r1") = (u16)entity->ext.skeleton.spinAngle; /* FAKEMATCH: pinned register */
    step += old; }
    entity->ext.skeleton.spinAngle = step; }
    { register s32 z asm("r1") = 0; ObjAffine_Set(entity->ext.skeleton.spinAngle, 0x10000, 0x10000, 0, z, (u16)entity->ext.skeleton.affine); } /* FAKEMATCH: pinned register */
    if (entity->state == 5)
        Skeleton_UpdateState5Animation(entity);
    else if (entity->state == 0xFF)
        Skeleton_UpdateDeath(entity);
    else if (Enemy_PreUpdate(entity) == 0)
    {
        entity->ext.skeleton.unusedDelay = RandomNumberGenerator() % 80 + 40;
        switch (entity->state)
        {
        case 0: Skeleton_UpdatePatrol(entity); break;
        case 1: Skeleton_UpdateCombatMovement(entity); break;
        case 2: Skeleton_UpdateIdle(entity); break;
        case 3: Skeleton_UpdateAttackPreparation(entity); break;
        case 4: Skeleton_UpdateThrow(entity); break;
        }
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}

