/* Dispatch Persephone's AI, death, reaction and display states. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Persephone_UpdateWalk(struct EwramData_EntityData *);
extern void Persephone_UpdateAttack(struct EwramData_EntityData *);
extern void Persephone_UpdatePreview(struct EwramData_EntityData *);
extern void Persephone_UpdateKnockdown(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyPersephoneUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) goto finished;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && entity->state != 2) goto collision_only;
    if (entity->hitFlashTimer && entity->state != 3 && entity->state != 2)
    {
collision_only:
        Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: Persephone_UpdateWalk(entity); break;
    case 1: Persephone_UpdateAttack(entity); break;
    case 2:
        entity->accelY.whole = 0x2800;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        break;
    case 4:
        Persephone_UpdatePreview(entity);
        Anim_Update(entity);
        goto finished;
    case 3: Persephone_UpdateKnockdown(entity); break;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
