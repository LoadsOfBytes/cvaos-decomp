#include "code_08009A0.h"
#include "code/code_08039340.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void KickerSkeleton_UpdateKick(struct EwramData_EntityData *);
extern s32 KickerSkeleton_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);

void EnemyKickerSkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0:
        if (entity->subState == 0)
        {
            if (!(RandomNumberGenerator() & 0x3F))
                Anim_StartIndexed(entity, RandomNumberGenerator() & 1, 0, 1);
            if (Target_IsNearEntity(entity, 0xC8, 0x50) || Enemy_IsDamaged(entity))
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
        }
        break;
    case 1: KickerSkeleton_UpdateKick(entity); break;
    case 2:
        if (KickerSkeleton_UpdateDeath(entity))
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
    if (entity->state != 2) Entity_AnimateWithBoxes(entity);
}
