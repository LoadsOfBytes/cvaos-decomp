#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 KyomaDemon_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Hit_EnableContact(struct EwramData_EntityData *, s32, void *, s32);
extern void KyomaDemon_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void KyomaDemon_OnHit(struct EwramData_EntityData *);
void EnemyKyomaDemonCreate(struct EwramData_EntityData *entity)
{
    struct EwramData **ewram = &gEwramData;
    struct BgLayerState *camera = &(*ewram)->bgInfo[1];
    s32 random;
    if (!KyomaDemon_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if ((*ewram)->gameState.bestiaryPreview) { entity->state = 3; return; }
    if (Anim_GetMaxBoxCount(entity) > 1) Hit_EnableContact(entity, 8, KyomaDemon_DamagePlayerOnContact, (s32)KyomaDemon_OnHit);
    else Hit_Enable(entity, 8, KyomaDemon_DamagePlayerOnContact, (s32)KyomaDemon_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->spriteLayer = 29;
    entity->state = 0;
    entity->ext.kyomaDemon.hidden = 1;
    entity->scrollWithBg = 0;
    random = RandomNumberGenerator() % 32;
    random += 32;
    entity->ext.kyomaDemon.lurkTimer = random;
    entity->ext.kyomaDemon.homeX = entity->posX.part16.integer + camera->xPos.part16.integer;
    entity->ext.kyomaDemon.homeY = entity->posY.part16.integer + camera->yPos.part16.integer;
    if (entity->spawnParam0 != 0)
    {
        entity->state = 2;
        entity->subState = 1;
        entity->hidden = 1;
    }
}
