/* Initialize a Ghost, a hidden controller or the display-mode animation. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern u8 Ghost_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Ghost_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Ghost_OnHit(struct EwramData_EntityData *);
void EnemyGhostCreate(struct EwramData_EntityData *entity)
{
    struct HitBoxRecord box = {-7, -7, 14, 14};
    if (entity->unk_514.part16.spawnParam1 != 0)
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        goto finished;
    }
    if (!Ghost_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 0, 0, 0);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        entity->posY.whole -= 0x240000;
        goto finished;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Ghost_DamagePlayerOnContact, Ghost_OnHit);
    else
        Hit_Enable(entity, 8, Ghost_DamagePlayerOnContact, (s32)Ghost_OnHit);
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    entity->hit.flags |= 2;
    entity->hit.flags |= 4;
    entity->state = 0;
    entity->flipX = Entity_IsTargetRight(entity);
finished:
    return;
}
