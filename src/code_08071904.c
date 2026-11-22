/* Dispatch Mudman states while preserving hit-flash and freeze update gates. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Mudman_UpdateWalk(struct EwramData_EntityData *);
extern void Mudman_UpdateAttack(struct EwramData_EntityData *);
extern void Mudman_UpdateDeath(struct EwramData_EntityData *);
extern void Mudman_UpdateHurt(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyMudmanUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2) goto collision_only;
    if (entity->unk_510.part8.hitFlashTimer && entity->state != 3 && entity->state != 2)
    {
collision_only:
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Mudman_UpdateWalk(entity); break;
    case 1: Mudman_UpdateAttack(entity); break;
    case 2: Mudman_UpdateDeath(entity); break;
    case 3: Mudman_UpdateHurt(entity); break;
    case 4: Anim_Update(entity); goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
