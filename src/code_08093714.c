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
 * count of its spawner (unk_500, unk_51A 0x39); the spawner (state 3)
 * stays at x 120 and with 1/32 chance creates a head below the limit
 * unk_514: x off the left or right room edge depending on Target_GetPixelX /
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
            parent = entity->unk_500.unk_500_entity;
            if (parent != NULL && parent->updateFunc != NULL && parent->unk_518.part8.enemyId == 0x39
                && parent->unk_4FC.whole != 0)
                parent->unk_4FC.whole--;
        }
        break;
    case 3:
        entity->posX.whole = 0x780000;
        if ((RandomNumberGenerator() & 0x1F) == 0 && (s32)entity->unk_4FC.whole < entity->unk_514.part16.spawnParam0)
        {
            head = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (head != NULL)
            {
                entity->unk_4FC.whole++;
                if ((s16)Target_GetPixelX(entity) <= 0x67)
                    head->posX.whole = 0x1000000;
                else if ((s16)Target_GetPixelX(entity) <= 0x88 && (u8)Target_GetFlipXOrOpposite(entity) == 0)
                    head->posX.whole = 0x1000000;
                else
                    head->posX.whole = -0x100000;
                x = (RandomNumberGenerator() & 0x3F) + 0x40;
                head->posY.part16.integer = x;
                head->scrollWithBg = 1;
                head->unk_518.part8.spawnIndex = 0;
                head->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
                head->unk_514.part16.spawnParam1 = entity->unk_514.part16.spawnParam1 & 1;
                head->drawFunc = (u32 *)Sprite_DrawEntity;
                head->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
                head->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
                head->updateFunc = entity->updateFunc;
                EnemyMedusaHeadCreate(head);
                head->unk_500.unk_500_entity = entity;
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
