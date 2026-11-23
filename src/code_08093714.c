#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_template.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern s32 Target_GetFlipXOrOpposite(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void MedusaHead_UpdateFly(struct EwramData_EntityData *entity);
extern void MedusaHead_UpdateIdle(struct EwramData_EntityData *entity);
extern void EntityUpdateNothing(void);
extern void EnemyMedusaHeadCreate(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 93714 | Medusa Head update: a head (states 0 / 1, MedusaHead_UpdateFly /
 * MedusaHead_UpdateIdle) dies off screen (state 2) and then decrements the head
 * count of its spawner (unk_500, enemyId 0x39); the spawner (state 3)
 * stays at x 120 and with 1/32 chance creates a head below the limit
 * spawnParam0: x off the left or right room edge depending on Target_GetPixelX /
 * Target_GetFlipXOrOpposite, y 64..127, template unk_C / unk_E from sEnemyData.
 */
void EnemyMedusaHeadUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent;
    struct EwramData_EntityData *head;
    s32 x;

    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2)
            Hit_CheckWithPlayer(collision, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        MedusaHead_UpdateFly(entity);
        break;
    case 1:
        MedusaHead_UpdateIdle(entity);
        break;
    case 2:
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            parent = entity->ext.medusaHead.spawner;
            if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x39
                && parent->ext.medusaHead.childCount != 0)
                parent->ext.medusaHead.childCount--;
        }
        break;
    case 3:
        entity->posX.whole = 0x780000;
        if ((RandomNumberGenerator() & 0x1F) == 0 && (s32)entity->ext.medusaHead.childCount < entity->spawnParam0)
        {
            head = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (head != NULL)
            {
                entity->ext.medusaHead.childCount++;
                if ((s16)Target_GetPixelX(entity) <= 0x67)
                    head->posX.whole = 0x1000000;
                else if ((s16)Target_GetPixelX(entity) <= 0x88 && (u8)Target_GetFlipXOrOpposite(entity) == 0)
                    head->posX.whole = 0x1000000;
                else
                    head->posX.whole = -0x100000;
                x = (RandomNumberGenerator() & 0x3F) + 0x40;
                head->posY.part16.integer = x;
                head->scrollWithBg = 1;
                head->spawnIndex = 0;
                head->enemyId = entity->enemyId;
                head->spawnParam1 = entity->spawnParam1 & 1;
                head->drawFunc = (u32 *)Sprite_DrawEntity;
                head->hp = sEnemyData[entity->enemyId].hp;
                head->mp = sEnemyData[entity->enemyId].mp;
                head->updateFunc = entity->updateFunc;
                EnemyMedusaHeadCreate(head);
                head->ext.medusaHead.spawner = entity;
            }
        }
        return;
    case 4:
        Anim_Update(entity);
        return;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}
