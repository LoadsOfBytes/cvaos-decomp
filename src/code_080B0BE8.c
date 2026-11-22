/* Minotaur update: pause handling, state dispatch, contact and animation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Entity_HideAboveY(struct EwramData_EntityData *, s32);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *, s32);
extern void Player_TrackEntity(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Music_SetCurrent(s32);
extern void Minotaur_UpdateIdle(struct EwramData_EntityData *);
extern void Minotaur_UpdateApproach(struct EwramData_EntityData *);
extern void Minotaur_UpdateAttack(struct EwramData_EntityData *);
extern void Werejaguar_UpdateDeath(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);

void Minotaur_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    Entity_HideAboveY(entity, -16);
    if (!EntityIsOutsideScreen(entity, 6))
        Player_TrackEntity(entity);
    if (entity->state != 3)
        Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 3)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (entity->unk_514.part16.spawnParam0 != 0 && !gEwramData->gameState.linkPlay)
            Music_SetCurrent(16);
        Minotaur_UpdateIdle(entity);
        break;
    case 1:
        Minotaur_UpdateApproach(entity);
        break;
    case 2:
        Minotaur_UpdateAttack(entity);
        break;
    case 3:
        Werejaguar_UpdateDeath(entity);
        break;
    case 4:
        Anim_Update(entity);
        return;
    }
    if (entity->state != 3)
        Hit_CheckWithPlayer(collision, entity);
    entity->unk_4F8.part8.unk_4F8 = entity->animStep;
    Entity_AnimateWithBoxes(entity);
}
