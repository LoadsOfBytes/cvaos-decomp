#include "code_08009A0.h"
#include "code/code_08039340.h"
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Tsuchinoko_UpdateJump(struct EwramData_EntityData *);
extern void Tsuchinoko_UpdateAttack(struct EwramData_EntityData *);
extern void Tsuchinoko_UpdateDeath(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);

void EnemyTsuchinokoUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2)
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Tsuchinoko_UpdateJump(entity); break;
    case 1: Tsuchinoko_UpdateAttack(entity); break;
    case 2: Tsuchinoko_UpdateDeath(entity); break;
    case 3: Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
