/* Initialize a Zombie or its hidden spawn controller. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Zombie_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Collision_IsSlopeTile(s16, s16);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Zombie_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Zombie_OnHit(struct EwramData_EntityData *);
void EnemyZombieCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    if (entity->spawnParam1 != 0)
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Entity_SnapToGround(entity);
        goto finished;
    }
    if (!Zombie_LoadGraphics(entity))
    {
        EntityDelete(entity);
        goto finished;
    }
    Anim_StartIndexed(entity, 0, 0, 0);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->state = 2;
        goto finished;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Zombie_DamagePlayerOnContact, Zombie_OnHit);
    else
        Hit_Enable(entity, 8, Zombie_DamagePlayerOnContact, Zombie_OnHit);
    Entity_UpdateBoxes(entity);
    {
        s32 previousY = entity->posY.whole;
        s32 zero;
        Entity_SnapToGround(entity);
        zero = 0;
        entity->state = zero;
        entity->flipX = Entity_IsTargetRight(entity);
        if (Collision_IsSlopeTile((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                         (s16)(background->yPos.part16.integer + entity->posY.part16.integer)))
        {
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
        if (entity->posY.whole <= previousY - 0x80000)
        {
            entity->state = 1;
            entity->timer = 1;
            entity->hp = zero;
        }
    }
finished:
    return;
}
