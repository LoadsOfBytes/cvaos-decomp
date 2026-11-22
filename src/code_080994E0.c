#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Ectoplasm_UpdateFloat(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Ectoplasm_UpdatePreview(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);

void EnemyEctoplasmUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (entity->state != 1)
    {
        if (EntityIsOutsideScreen(entity, 4)) return;
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 1) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 1)
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Ectoplasm_UpdateFloat(entity); break;
    case 1: EntityAccelerateAndMove(entity, 0x40000); Enemy_UpdateFragmentDeath(entity); break;
    case 2: Ectoplasm_UpdatePreview(entity); Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
