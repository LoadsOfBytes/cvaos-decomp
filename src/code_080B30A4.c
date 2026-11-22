#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void BomberRockArmor_UpdateWalk(struct EwramData_EntityData *);
extern void RockArmor_UpdateAttack(struct EwramData_EntityData *);
extern s32 BomberRockArmor_UpdateDeath(struct EwramData_EntityData *);

void EnemyRockArmorUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: BomberRockArmor_UpdateWalk(entity); break;
    case 1: RockArmor_UpdateAttack(entity); break;
    case 2:
        if (BomberRockArmor_UpdateDeath(entity))
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 3: Anim_Update(entity); return;
    }
    if (entity->state != 2) Hit_CheckWithPlayer(box, entity);
    Entity_AnimateWithBoxes(entity);
}
