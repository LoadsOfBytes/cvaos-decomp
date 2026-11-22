/* Common update gates and palette changes after an enemy takes damage. */
#include "structs/ewram.h"
#include "structs/entity_template.h"

s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);

static inline s32 TemplateBlocksUpdate(struct EwramData_EntityData *entity)
{
    u32 base = (u32)sEnemyData;
    u8 index = entity->unk_518.part8.enemyId;
    struct EntityTemplate *entry = (struct EntityTemplate *)(index * sizeof(*entry) + base);
    if (entry->flags_1C & 0x400)
        return 0;
    return 1;
}

s32 Enemy_IsStunned(struct EwramData_EntityData *entity)
{
    if ((s16)entity->unk_518.part16.hp <= 0)
        return 0;
    if (Enemy_UpdateHitFlash(entity))
        return 1;
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0)
    {
        if (TemplateBlocksUpdate(entity))
            return 1;
    }
    if (0x100 & *(u16 *)&entity->unk_51C)
    {
        entity->palette = 14;
        entity->unk_510.part8.basePalette = entity->palette;
        return 1;
    }
    return 0;
}

s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *entity)
{
    if ((s16)entity->unk_518.part16.hp <= 0)
        return 0;
    if (entity->unk_510.part8.hitFlashTimer != 0)
        return 1;
    if (!gEwramData->gameState.bestiaryPreview && gEwramData->gameState.timeStopTimer != 0)
    {
        if (TemplateBlocksUpdate(entity))
            return 1;
    }
    if (0x100 & *(u16 *)&entity->unk_51C)
    {
        entity->palette = 14;
        entity->unk_510.part8.basePalette = entity->palette;
        return 1;
    }
    return 0;
}

/* Count down the hit flash, alternating between palette 4 and the saved
 * palette on bit 1 of the timer. */
s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity)
{
    if (entity->unk_510.part8.hitFlashTimer != 0)
    {
        if (--entity->unk_510.part8.hitFlashTimer & 2)
            entity->palette = 4;
        else
            entity->palette = entity->unk_510.part8.basePalette;
        return 1;
    }
    return 0;
}
