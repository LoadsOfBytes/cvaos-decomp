/* Werewolf update dispatcher and jump state. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"

extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Werewolf_UpdateWalk(struct EwramData_EntityData *);
extern void Werewolf_UpdateAttack(struct EwramData_EntityData *);
extern void Werejaguar_UpdateDeath(struct EwramData_EntityData *);
extern void Werewolf_UpdateRecoil(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);

void EnemyWerewolfUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4))
            return;
        if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
            Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4))
        entity->hidden = 1;
    else
        entity->hidden = 0;
    if (entity->state != 2)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity))
    {
        if (entity->state != 2)
        {
            Hit_CheckWithPlayer(box, entity);
            return;
        }
    }
    if (entity->hitFlashTimer != 0 && entity->state != 3 && entity->state != 2)
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: Werewolf_UpdateWalk(entity); break;
    case 1: Werewolf_UpdateAttack(entity); break;
    case 2: Werejaguar_UpdateDeath(entity); break;
    case 3: Werewolf_UpdateRecoil(entity); break;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
