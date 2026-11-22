/* EnemySkeletonKnightCreate 0x0808EC64..0x0808ECF7. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 SkeletonKnight_LoadGraphics(struct EwramData_EntityData *entity);
extern void SkeletonKnight_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void SkeletonKnight_OnHit(struct EwramData_EntityData *param_0);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 8EC64 | Skeleton Knight create: load the graphics (delete the
 * enemy when that fails), start the animation and set up the hit boxes
 * (Hit_EnableContact when a box record has a value > 1 at offset 4, else
 * Hit_Enable); state 3 instead when bestiaryPreview is set.
 */
void EnemySkeletonKnightCreate(struct EwramData_EntityData *entity)
{
    if (!SkeletonKnight_LoadGraphics(entity))
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
        Hit_EnableContact(entity, 8, SkeletonKnight_DamagePlayerOnContact, (s32)SkeletonKnight_OnHit);
    else
        Hit_Enable(entity, 8, SkeletonKnight_DamagePlayerOnContact, (s32)SkeletonKnight_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}
