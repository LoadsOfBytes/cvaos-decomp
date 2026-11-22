/* Werejaguar dispatch, collision handling and animation change notification. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void Werejaguar_UpdateWalk(struct EwramData_EntityData *);
extern void Werejaguar_UpdateAttack(struct EwramData_EntityData *);
extern void Werejaguar_UpdateDeath(struct EwramData_EntityData *);
extern void Werejaguar_UpdateRecoil(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyWerejaguarUpdate(struct EwramData_EntityData *entity)
{
    s32 bounds[12];
    if(entity->state!=2)
    {
        if(EntityIsOutsideScreen(entity,4)) return;
        if(Entity_IsInRect(entity,-16,0,272,176)) Player_TrackEntity(entity);
    }
    if(EntityIsOutsideScreen(entity,4)) entity->hidden=1;
    else entity->hidden=0;
    entity->unk_4F8.part8.unk_4F9=0;
    entity->unk_4F8.part8.unk_4F8=0;
    entity->animFlags&=0xFD;
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
        case 0: Werejaguar_UpdateWalk(entity); break;
        case 1: Werejaguar_UpdateAttack(entity); break;
        case 2: Werejaguar_UpdateDeath(entity); break;
        case 3: Werejaguar_UpdateRecoil(entity); break;
        case 4: Anim_Update(entity); return;
    }
    if((s8)entity->unk_4F8.part8.unk_4F8==0)
    {
        u8 frame=entity->animFrame;
        Anim_Update(entity);
        if(frame!=entity->animFrame) Entity_UpdateBoxes(entity);
    }
    Enemy_CheckHitPlayer(entity);
}
