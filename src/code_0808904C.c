/* Initialize the shared Gorgon/Catoblepas body and its collision callbacks. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Gorgon_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Gorgon_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Gorgon_OnHit(struct EwramData_EntityData *);
void EnemyGorgonCreate(struct EwramData_EntityData *entity)
{
    if (!Gorgon_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 9;
    if (gEwramData->gameState.bestiaryPreview) entity->state = 3;
    else
    {
        if (Anim_GetMaxBoxCount(entity) > 1)
            Hit_EnableContact(entity, 8, Gorgon_DamagePlayerOnContact, Gorgon_OnHit);
        else
            Hit_Enable(entity, 8, Gorgon_DamagePlayerOnContact, Gorgon_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->state = 0;
    }
finished:
    return;
}
