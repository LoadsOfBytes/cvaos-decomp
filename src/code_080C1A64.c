/* Altair child creation and feather motion. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/entity_template.h"

/**
 * @brief Create the selected enemy type below Altair, set its parameters
 * and call its template create callback. Return nonzero if it remains
 * active; update linked spawner counters using the existing rules.
 */
s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *entity, u8 type, u16 spawnParam0, u16 spawnParam1)
{
    u8 result = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
    entity->ext.altair.carried = child;
    if (child != NULL)
    {
        struct EntityTemplate *entry;
        struct EwramData_EntityData *parent;
        s32 x;
        entity->ext.altair.carrying = 1;
        x = entity->posX.whole;
        child->posX.whole = x;
        child->posY.whole = entity->posY.whole + 0x130000;
        child->flipX = x < gEwramData->playerState.player->posX.whole;
        child->scrollWithBg = 1;
        child->spawnParam0 = spawnParam0;
        child->spawnParam1 = spawnParam1;
        child->spawnIndex = entity->spawnIndex;
        child->enemyId = type;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        entry = (struct EntityTemplate *)(type * sizeof(*entry) + (u32)sEnemyData);
        child->hp = entry->hp;
        child->updateFunc = entry->updateFunc;
        entry->createFunc(child);
        result = child->updateFunc != NULL;
        parent = entity->ext.altair.spawner;
        if (parent != NULL)
        {
            parent->ext.altairDrop.count++;
            child->ext.altairDrop.count++;
            child->ext.altairDrop.spawnerIndex = (u16)ENTITY_INDEX_PAIR(entity->ext.altair.spawner);
        }
    }
    return result;
}

