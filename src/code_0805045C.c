/* Conditional creation from the common entity template table. */
#include "structs/ewram.h"
#include "structs/entity_template.h"
#include "code_08009A0.h"
#include "code_08011DD0.h"

void Object0ACreate(struct EwramData_EntityData *entity)
{
    if (((gEwramData->gameState.bossEventFlags >> entity->spawnParam1) & 1) == 0)
        EntityDelete(entity);
    else
    {
        struct EntityTemplate *entry;
        entity->enemyId = entity->spawnParam0;
        entry = (struct EntityTemplate *)(entity->enemyId * sizeof(*entry) + (u32)sEnemyData);
        entity->hp = entry->hp;
        entity->spawnParam0 = 0;
        entity->spawnParam1 = 0;
        entity->updateFunc = entry->updateFunc;
        entry->createFunc(entity);
    }
}

void Object0AUpdate(struct EwramData_EntityData *entity)
{
}

void Object0BCreate(struct EwramData_EntityData *entity)
{
    if (!GameFlag_GetMask(entity->spawnParam1))
        EntityDelete(entity);
    else
    {
        struct EntityTemplate *entry;
        entity->enemyId = entity->spawnParam0;
        entry = (struct EntityTemplate *)(entity->enemyId * sizeof(*entry) + (u32)sEnemyData);
        entity->hp = entry->hp;
        entity->spawnParam0 = 0;
        entity->spawnParam1 = 0;
        entity->updateFunc = entry->updateFunc;
        entry->createFunc(entity);
    }
}

void Object0BUpdate(struct EwramData_EntityData *entity)
{
}
