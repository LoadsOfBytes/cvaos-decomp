/* Common update gates and palette changes after an enemy takes damage. */
#include "structs/ewram.h"
#include "enemy_status.h"
#include "structs/entity_template.h"

s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);

static inline s32 TemplateBlocksUpdate(struct EwramData_EntityData *entity)
{
    u32 base = (u32)sEnemyData;
    u8 index = entity->enemyId;
    struct EntityTemplate *entry = (struct EntityTemplate *)(index * sizeof(*entry) + base);
    if (entry->flags_1C & 0x400)
        return 0;
    return 1;
}

s32 Enemy_IsStunned(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp <= 0)
        return 0;
    if (Enemy_UpdateHitFlash(entity))
        return 1;
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0)
    {
        if (TemplateBlocksUpdate(entity))
            return 1;
    }
    if (ENEMY_STATUS_PETRIFIED & ENEMY_STATUS(entity))
    {
        entity->palette = 14;
        entity->basePalette = entity->palette;
        return 1;
    }
    return 0;
}

s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *entity)
{
    if ((s16)entity->hp <= 0)
        return 0;
    if (entity->hitFlashTimer != 0)
        return 1;
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0)
    {
        if (TemplateBlocksUpdate(entity))
            return 1;
    }
    if (ENEMY_STATUS_PETRIFIED & ENEMY_STATUS(entity))
    {
        entity->palette = 14;
        entity->basePalette = entity->palette;
        return 1;
    }
    return 0;
}

/* Count down the hit flash, alternating between palette 4 and the saved
 * palette on bit 1 of the timer. */
s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity)
{
    if (entity->hitFlashTimer != 0)
    {
        if (--entity->hitFlashTimer & 2)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        return 1;
    }
    return 0;
}
