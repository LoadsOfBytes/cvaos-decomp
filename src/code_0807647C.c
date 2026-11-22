/* Harpy/Siren dispatch share movement states and collision processing. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Harpy_UpdateApproach(struct EwramData_EntityData *);
extern void Harpy_UpdateAttack(struct EwramData_EntityData *);
extern void Harpy_UpdateDeath(struct EwramData_EntityData *);
extern void Harpy_UpdateRecoil(struct EwramData_EntityData *);
extern void EnemyHarpyCreate(struct EwramData_EntityData *);
void EnemyHarpyUpdate(struct EwramData_EntityData *entity)
{
    s32 scratch[4];
    if(Enemy_PreUpdate(entity)) return;
    switch(entity->state)
    {
        case 0: Harpy_UpdateApproach(entity); break;
        case 1: Harpy_UpdateAttack(entity); break;
        case 2: Harpy_UpdateDeath(entity); break;
        case 3: Harpy_UpdateRecoil(entity); break;
        case 4: Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
void EnemySirenUpdate(struct EwramData_EntityData *entity)
{
    EnemyHarpyUpdate(entity);
}
void EnemySirenCreate(struct EwramData_EntityData *entity)
{
    EnemyHarpyCreate(entity);
}
