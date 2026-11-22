#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void SkullArcher_UpdateIdle(struct EwramData_EntityData *);
extern void SkullArcher_UpdateShoot(struct EwramData_EntityData *);
extern void SkullArcher_UpdatePatrol(struct EwramData_EntityData *);
extern void SkullArcher_SpawnDeathPieces(struct EwramData_EntityData *);

void EnemySkullArcherUpdate(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: SkullArcher_UpdateIdle(entity); break;
    case 1: SkullArcher_UpdateShoot(entity); break;
    case 2: SkullArcher_UpdatePatrol(entity); break;
    case 3: SkullArcher_SpawnDeathPieces(entity); EntityDelete(entity); return;
    case 4: Anim_Update(entity); return;
    }
    Enemy_CheckHitPlayer(entity);
    Entity_AnimateWithBoxes(entity);
}

