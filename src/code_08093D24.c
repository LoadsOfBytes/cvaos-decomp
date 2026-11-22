#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Succubus_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Succubus_DamagePlayerOnContact(void);
extern void Succubus_OnHit(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

void EnemySuccubusCreate(struct EwramData_EntityData *entity)
{
    if (!Succubus_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 10, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        Anim_StartIndexed(entity, 1, 0, 1);
        Anim_Update(entity);
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Succubus_DamagePlayerOnContact, Succubus_OnHit);
    else
        Hit_Enable(entity, 8, Succubus_DamagePlayerOnContact, Succubus_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->spriteLayer = 9;
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        entity->state = 0;
        entity->unk_4FC.part8.unk_4FE = 1;
        entity->subState = 0;
    }
    else
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->subState = 2;
        entity->unk_4FC.part8.unk_4FE = 0;
        entity->unk_4FC.part8.unk_4FF = 0;
    }
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x1800;
}
