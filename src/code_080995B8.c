#include "code_08009A0.h"
#include "ectoplasm_entity.h"
#include "code/code_08039340.h"
extern u8 Ectoplasm_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Hit_Enable(struct EwramData_EntityData *, s32, void *, void *);
extern void Ectoplasm_DamagePlayerOnContact(void);
extern void Ectoplasm_OnHit(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);

void EnemyEctoplasmCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    struct BgLayerState *background = &(*ewram)->bgInfo[1];
    if (!Ectoplasm_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if ((*ewram)->gameState.bestiaryPreview)
    {
        ECTOPLASM_POINT_X(entity) = entity->posX.whole + background->xPos.whole;
        ECTOPLASM_POINT_Y(entity) = entity->posY.whole + background->yPos.whole;
        entity->state = 2;
        return;
    }
    entity->spriteLayer = 3;
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, Ectoplasm_DamagePlayerOnContact, Ectoplasm_OnHit);
    else Hit_Enable(entity, 8, Ectoplasm_DamagePlayerOnContact, Ectoplasm_OnHit);
    Entity_UpdateBoxes(entity);
    entity->state = 0;
    ECTOPLASM_POINT_X(entity) = entity->posX.whole + background->xPos.whole;
    ECTOPLASM_POINT_Y(entity) = entity->posY.whole + background->yPos.whole;
    entity->scrollWithBg = 1;
    entity->accelY.whole = -0x100;
    entity->timer = 0x1E;
}
