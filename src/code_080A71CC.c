#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Tsuchinoko_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Tsuchinoko_DamagePlayerOnContact(void);
extern void Tsuchinoko_OnHit(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

void EnemyTsuchinokoCreate(struct EwramData_EntityData *entity)
{
    if (!Tsuchinoko_LoadGraphics(entity))
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
        Hit_EnableContact(entity, 8, Tsuchinoko_DamagePlayerOnContact, Tsuchinoko_OnHit);
    else
        Hit_Enable(entity, 8, Tsuchinoko_DamagePlayerOnContact, Tsuchinoko_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->ext.tsuchinoko.maxHp = entity->hp;
    {
        u32 random = RandomNumberGenerator();
        u32 mask = 0x7F;
        mask &= random;
        if (mask > 0x18)
        {
            entity->subState = 12;
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->animTimer = 0;
            entity->animStep = 8;
            Anim_Update(entity);
        }
    }
}
