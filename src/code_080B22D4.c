#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void DiscArmor_UpdateWalk(struct EwramData_EntityData *);
extern void DiscArmor_UpdateThrow(struct EwramData_EntityData *);
extern s32 DiscArmor_UpdateDeath(struct EwramData_EntityData *);

void EnemyDiscArmorUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 y = entity->unk_504.part16.unk_506[0];
    s32 x = (s16)entity->unk_504.part16.unk_504;
    if (entity->flipX) x = (u16)-x;
    entity->unk_4FC.whole = entity->posX.whole + (x << 16);
    entity->unk_500.whole = entity->posY.whole + (y << 16);
    if (Enemy_PreUpdate(entity)) return;
    switch (entity->state)
    {
    case 0: DiscArmor_UpdateWalk(entity); break;
    case 1: DiscArmor_UpdateThrow(entity); break;
    case 2:
        if (DiscArmor_UpdateDeath(entity))
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
