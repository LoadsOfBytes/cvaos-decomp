/* Altair child creation and feather motion. */
#include "code_08009A0.h"
#include "altair_entity.h"
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
    ALTAIR_CARRIED(entity) = child;
    if (child != NULL)
    {
        struct EntityTemplate *entry;
        struct EwramData_EntityData *parent;
        s32 x;
        ALTAIR_CARRYING(entity) = 1;
        x = entity->posX.whole;
        child->posX.whole = x;
        child->posY.whole = entity->posY.whole + 0x130000;
        child->flipX = x < gEwramData->playerState.player->posX.whole;
        child->scrollWithBg = 1;
        child->unk_514.part16.spawnParam0 = spawnParam0;
        child->unk_514.part16.spawnParam1 = spawnParam1;
        child->unk_518.part8.spawnIndex = entity->unk_518.part8.spawnIndex;
        child->unk_518.part8.enemyId = type;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        entry = (struct EntityTemplate *)(type * sizeof(*entry) + (u32)sEnemyData);
        child->unk_518.part16.hp = entry->hp;
        child->updateFunc = entry->updateFunc;
        entry->createFunc(child);
        result = child->updateFunc != NULL;
        parent = ALTAIR_SPAWNER(entity);
        if (parent != NULL)
        {
            ALTAIR_DROP_COUNT(parent)++;
            ALTAIR_DROP_COUNTED(child)++;
            ALTAIR_DROP_SPAWNER_INDEX(child) = (u16)ENTITY_INDEX_PAIR(ALTAIR_SPAWNER(entity));
        }
    }
    return result;
}

