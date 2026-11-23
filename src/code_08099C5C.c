/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyBiphronCreate = EnemySkeletonKnightCreate */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Biphron_LoadGraphics(struct EwramData_EntityData *entity);
extern void Biphron_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Biphron_OnHit(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 99C5C | Same as EnemySkeletonKnightCreate.
 */
void EnemyBiphronCreate(struct EwramData_EntityData *entity)
{
    if (!Biphron_LoadGraphics(entity))
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
        Hit_EnableContact(entity, 8, Biphron_DamagePlayerOnContact, (s32)Biphron_OnHit);
    else
        Hit_Enable(entity, 8, Biphron_DamagePlayerOnContact, (s32)Biphron_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}
