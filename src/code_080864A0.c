/* Decompiled locally from the original assembly (variant of an already
 * decompiled function). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Gargoyle_LoadGraphics(struct EwramData_EntityData *entity);
extern void Gargoyle_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Gargoyle_OnHit(struct EwramData_EntityData *param_0);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 864A0 | Gargoyle create: like EnemyValkyrieCreate, but state 2 when
 * bestiaryPreview is set.
 */
void EnemyGargoyleCreate(struct EwramData_EntityData *entity)
{
    if (!Gargoyle_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Gargoyle_DamagePlayerOnContact, (s32)Gargoyle_OnHit);
    else
        Hit_Enable(entity, 8, Gargoyle_DamagePlayerOnContact, (s32)Gargoyle_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
}
