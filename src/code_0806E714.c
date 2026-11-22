/* EnemyValkyrieCreate 0x0806E714..0x0806E7A7. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Valkyrie_LoadGraphics(struct EwramData_EntityData *entity);
extern void Valkyrie_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Valkyrie_OnHit(struct EwramData_EntityData *param_0);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 6E714 | Valkyrie create: load the graphics (delete the
 * enemy when that fails), start the animation and set up the hit boxes
 * (Hit_EnableContact when a box record has a value > 1 at offset 4, else
 * Hit_Enable), layer 3; state 4 instead when bestiaryPreview is set.
 */
void EnemyValkyrieCreate(struct EwramData_EntityData *entity)
{
    if (!Valkyrie_LoadGraphics(entity))
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
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Valkyrie_DamagePlayerOnContact, (s32)Valkyrie_OnHit);
    else
        Hit_Enable(entity, 8, Valkyrie_DamagePlayerOnContact, (s32)Valkyrie_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
}
