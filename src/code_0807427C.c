#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"

extern s32 Une_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt);
extern void Une_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Une_OnHit(struct EwramData_EntityData *entity);
extern void EntityUpdateNothing(void);

/**
 * @brief 7427C | Une create: deleted when Une_LoadGraphics fails; frozen time
 * (bestiaryPreview) gives state 4 with animation 3; otherwise Entity_EnableBoxes
 * callbacks, and when spawnParam1 is set (also after Entity_SnapToGround) it becomes
 * the waiting state 3 hidden and without boxes, creating a child Une of
 * the same template that it counts in unk_4FC (child unk_500 = 1); else
 * state 0 with timer 60. accelY = 0x2800.
 */
void EnemyUneCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    if ((u8)Une_LoadGraphics(entity) == 0)
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        Anim_StartIndexed(entity, 3, 0, 1);
        Anim_Update(entity);
        return;
    }
    Entity_EnableBoxes(entity, Une_DamagePlayerOnContact, Une_OnHit);
    if (entity->spawnParam1 != 0)
    {
        Entity_SnapToGround(entity);
        if (entity->spawnParam1 != 0)
        {
            entity->state = 3;
            entity->hidden = 1;
            entity->hit.flags |= 4;
            entity->hit.flags |= 2;
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->scrollWithBg = 1;
                child->spawnIndex = 0;
                child->enemyId = entity->enemyId;
                child->spawnParam0 = 0;
                child->spawnParam1 = 0;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                child->hp = sEnemyData[entity->enemyId].hp;
                child->mp = sEnemyData[entity->enemyId].mp;
                child->updateFunc = entity->updateFunc;
                EnemyUneCreate(child);
                child->accelY.whole = 0x2800;
                child->ext.une.spawner = entity;
                entity->ext.uneSpawner.count = 1;
                child->ext.une.original = 1;
            }
            goto end;
        }
    }
    entity->state = 0;
    entity->timer = 0x3C;
end:
    entity->accelY.whole = 0x2800;
}
