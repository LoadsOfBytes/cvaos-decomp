#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern void WaiterSkeleton_UpdateThrow(struct EwramData_EntityData *);
extern void WaiterSkeleton_UpdateWalk(struct EwramData_EntityData *);
extern s32 WaiterSkeleton_SpawnDeathPieces(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);

void EnemyWaiterSkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0:
        Entity_FaceTarget(entity);
        if (entity->subState == 0
            && Target_IsInRect((s16)(entity->posX.part16.integer - 0xA0),
                            (s16)(entity->posY.part16.integer - 0x28), 0x140, 0x50))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1: WaiterSkeleton_UpdateThrow(entity); break;
    case 2: WaiterSkeleton_UpdateWalk(entity); break;
    case 3:
        if (WaiterSkeleton_SpawnDeathPieces(entity)) EntityDelete(entity);
        return;
    case 4:
        Anim_Update(entity);
        return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}
