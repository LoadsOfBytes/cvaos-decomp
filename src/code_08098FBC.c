/* Mimic initialization, including the dormant and alternate room-start paths. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 Mimic_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void Mimic_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Mimic_OnHit(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Mimic_SpawnTrigger(struct EwramData_EntityData *, s32, s32);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
void EnemyMimicCreate(struct EwramData_EntityData *entity)
{
    u32 box = 0x0E10F2F8;
    if (!Mimic_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Enemy_ResetFromTemplate(entity);
    entity->flipX = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(entity);
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1) Hit_EnableContact(entity, 8, Mimic_DamagePlayerOnContact, (s32)Mimic_OnHit);
    else Hit_Enable(entity, 8, Mimic_DamagePlayerOnContact, (s32)Mimic_OnHit);
    if (entity->spawnParam0 != 0)
    {
        entity->ext.mimic.flags |= 2;
        entity->ext.mimic.heldFrame = 0;
        entity->animFrame = 0;
        HitBox_SetBody(entity, &box);
        HitBox_SetAttack(entity, &box);
        entity->hit.flags &= ~4;
        entity->hit.flags |= 2;
        entity->state = 0;
        entity->subState = 3;
        Entity_SnapToGround(entity);
    }
    else
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        Anim_Update(entity);
        entity->hidden = 1;
        Mimic_SpawnTrigger(entity, 0, 0);
        Entity_UpdateBoxes(entity);
        entity->state = 0;
        entity->subState = 0;
    }
}
