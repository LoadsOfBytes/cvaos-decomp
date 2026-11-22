/* EnemyWerejaguarCreate. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Werejaguar_LoadGraphics(struct EwramData_EntityData *entity);
extern void Werejaguar_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Werejaguar_OnHit(struct EwramData_EntityData *param_0);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 796F0 | Werejaguar create: load the graphics (delete the enemy
 * when that fails), start the animation and set up the hit boxes; state 4
 * instead when bestiaryPreview is set.
 */
void EnemyWerejaguarCreate(struct EwramData_EntityData *entity)
{
    if (!Werejaguar_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Hit_EnableContact(entity, 8, Werejaguar_DamagePlayerOnContact, (s32)Werejaguar_OnHit);
    Entity_UpdateBoxes(entity);
    Hit_SetTimerOverride(3, 6);
    Entity_SnapToGround(entity);
    entity->state = 0;
}
