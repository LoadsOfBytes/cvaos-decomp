/* Altair child creation and feather motion. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/entity_template.h"

/**
 * @brief Create the selected enemy type below Altair, set its parameters
 * and call its template create callback. Return nonzero if it remains
 * active; update linked spawner counters using the existing rules.
 */
s32 Altair_CreateCarriedEnemy(struct EwramData_EntityData *entity, u8 type, u16 param_2, u16 param_3)
{
    u8 result = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
    entity->unk_4F8.unk_4F8_entity = child;
    if (child != NULL)
    {
        struct EntityTemplate *entry;
        struct EwramData_EntityData *parent;
        s32 x;
        entity->unk_500.part8.unk_500 = 1;
        x = entity->posX.whole;
        child->posX.whole = x;
        child->posY.whole = entity->posY.whole + 0x130000;
        child->flipX = x < gEwramData->playerState.player->posX.whole;
        child->scrollWithBg = 1;
        child->unk_514.part16.spawnParam0 = param_2;
        child->unk_514.part16.spawnParam1 = param_3;
        child->unk_518.part8.spawnIndex = entity->unk_518.part8.spawnIndex;
        child->unk_518.part8.enemyId = type;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        entry = (struct EntityTemplate *)(type * sizeof(*entry) + (u32)sEnemyData);
        child->unk_518.part16.hp = entry->hp;
        child->updateFunc = entry->updateFunc;
        entry->createFunc(child);
        result = child->updateFunc != NULL;
        parent = entity->unk_504.unk_504_entity;
        if (parent != NULL)
        {
            parent->unk_4FC.part8.unk_4FD++;
            child->unk_4FC.part8.unk_4FD++;
            child->unk_4F4.part16.unk_4F6 = (u16)entity->unk_504.unk_504_entity->unk_4F4.part16.unk_4F4;
        }
    }
    return result;
}

