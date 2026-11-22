#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Lilith_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Lilith_DamagePlayerOnContact(void);
extern void Lilith_OnHit(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

void EnemyLilithCreate(struct EwramData_EntityData *entity)
{
    if (!Lilith_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        Anim_StartIndexed(entity, 5, 0, 1);
        Anim_Update(entity);
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Lilith_DamagePlayerOnContact, Lilith_OnHit);
    else
        Hit_Enable(entity, 8, Lilith_DamagePlayerOnContact, Lilith_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->hidden = 1;
    entity->unk_4FC.part8.unk_4FF = 1;
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x1800;
}
