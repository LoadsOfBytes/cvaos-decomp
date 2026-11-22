/* Arachne dispatch and screen-dependent sound and collision updates. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Sound_StopTracked(s32);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Arachne_UpdateWalk(struct EwramData_EntityData *);
extern void Arachne_UpdateAttack(struct EwramData_EntityData *);
extern void Arachne_UpdateRecoil(struct EwramData_EntityData *);
extern void Arachne_UpdateDeath(struct EwramData_EntityData *);
extern void Arachne_UpdatePreview(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyArachneUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state!=2)
    {
        if(EntityIsOutsideScreen(entity,4)) { Sound_StopTracked(0x82); return; }
        if(Entity_IsInRect(entity,-16,0,272,176)) Player_TrackEntity(entity);
    }
    if(EntityIsOutsideScreen(entity,4)) entity->hidden=1;
    else entity->hidden=0;
    if(entity->state!=2) Enemy_UpdateHitFlash(entity);
    if(Entity_IsTimeStopped(entity) && entity->state!=2) goto collision;
    if(entity->unk_510.part8.hitFlashTimer && entity->state!=3 && entity->state!=2)
    {
collision:
        Hit_CheckWithPlayer(bounds,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: Arachne_UpdateWalk(entity); break;
        case 1: Arachne_UpdateAttack(entity); break;
        case 3: Arachne_UpdateRecoil(entity); break;
        case 2: Arachne_UpdateDeath(entity); break;
        case 4: Arachne_UpdatePreview(entity); Anim_Update(entity); return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
