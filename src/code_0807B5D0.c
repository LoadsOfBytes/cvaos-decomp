/* Initialize Persephone's graphics, collisions and initial facing. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Persephone_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Persephone_OnHit(struct EwramData_EntityData *);
void EnemyPersephoneCreate(struct EwramData_EntityData *entity)
{
    if (!Persephone_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        goto finished;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Persephone_DamagePlayerOnContact, Persephone_OnHit);
    else
        Hit_Enable(entity, 8, Persephone_DamagePlayerOnContact, Persephone_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->flipX = Entity_IsTargetRight(entity);
    entity->state = 0;
    ENTITY_OWNER(entity) = entity;
finished:
    return;
}
