#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Curly_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Curly_DamagePlayerOnContact(void);
extern void Curly_OnHit(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

void EnemyCurlyCreate(struct EwramData_EntityData *entity)
{
    if (!Curly_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Curly_DamagePlayerOnContact, Curly_OnHit);
    else
        Hit_Enable(entity, 8, Curly_DamagePlayerOnContact, Curly_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x2800;
}
