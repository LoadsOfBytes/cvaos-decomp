#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 WingedSkeleton_LoadGraphics(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void WingedSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void WingedSkeleton_OnHit(struct EwramData_EntityData *);
void EnemyWingedSkeletonCreate(struct EwramData_EntityData *entity)
{
    if (!WingedSkeleton_LoadGraphics(entity)) { EntityDelete(entity); return; }
    Anim_StartIndexed(entity, 2, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview) { entity->state = 4; return; }
    entity->spriteLayer = 3;
    Hit_Enable(entity, 8, WingedSkeleton_DamagePlayerOnContact, (s32)WingedSkeleton_OnHit);
    Entity_UpdateBoxes(entity);
    if (entity->spawnParam1 != 0)
    {
        entity->state = 3;
        entity->hidden = 1;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
    }
    else
    {
        entity->state = 0;
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX) entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) + 0xC000;
        else entity->velX.whole = -0xC000 - (RandomNumberGenerator() & 0x3FFF);
    }
}
