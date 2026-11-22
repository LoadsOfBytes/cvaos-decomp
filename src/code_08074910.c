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
extern void WingedSkeleton_UpdateFly(struct EwramData_EntityData *entity);
extern void WingedSkeleton_UpdateAttack(struct EwramData_EntityData *entity);
extern void WingedSkeleton_SpawnDeathPieces(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void EntityUpdateNothing(void);
extern void EnemyWingedSkeletonCreate(struct EwramData_EntityData *entity);
extern s32 Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);

/**
 * @brief 74910 | Winged Skeleton update (as EnemyMedusaHeadUpdate): a
 * skeleton (states 0 / 1, WingedSkeleton_UpdateFly / WingedSkeleton_UpdateAttack) dies off screen
 * (state 2, WingedSkeleton_SpawnDeathPieces when HP is gone) and then decrements the count of
 * its spawner (unk_4FC, unk_51A 0x14); the spawner (state 3) with 1/32
 * chance creates one below the limit unk_514: x off the room edge farther
 * from the player, y above / below / in the view depending on the vertical
 * camera speed of bgInfo[1].
 */
void EnemyWingedSkeletonUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent;
    struct EwramData_EntityData *head;
    s32 x;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

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
        WingedSkeleton_UpdateFly(entity);
        break;
    case 1:
        WingedSkeleton_UpdateAttack(entity);
        break;
    case 2:
        entity->timer = 1;
        if ((s16)entity->unk_518.part16.hp <= 0)
            WingedSkeleton_SpawnDeathPieces(entity);
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            parent = entity->unk_4FC.unk_4FC_entity;
            if (parent != NULL && parent->updateFunc != NULL && parent->unk_518.part8.enemyId == 0x14
                && parent->unk_4FC.whole != 0)
                parent->unk_4FC.whole--;
        }
        break;
    case 3:
        if ((RandomNumberGenerator() & 0x1F) == 0 && (s32)entity->unk_4FC.whole < entity->unk_514.part16.spawnParam0)
        {
            head = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (head != NULL)
            {
                entity->unk_4FC.whole++;
                if (gEwramData->playerState.player->posX.whole <= 0x1FFFFF)
                    head->posX.whole = 0x1000000;
                else if (gEwramData->playerState.player->posX.whole <= 0xD00000 && (RandomNumberGenerator() & 1))
                    head->posX.whole = 0x1000000;
                else
                    head->posX.whole = -0x100000;
                if (bg->yVel.whole < 0)
                    head->posY.part16.integer = RandomNumberGenerator() % 0x50 - 0x40;
                else
                {
                    if (bg->yVel.whole > 0)
                        x = (RandomNumberGenerator() & 0x3F) + 0x90;
                    else
                        x = (RandomNumberGenerator() & 0x3F) + 0x30;
                    head->posY.part16.integer = x;
                }
                head->scrollWithBg = 1;
                head->unk_518.part8.spawnIndex = 0;
                head->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
                head->unk_514.part16.spawnParam1 = 0;
                head->drawFunc = (u32 *)Sprite_DrawEntity;
                head->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
                head->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
                head->updateFunc = entity->updateFunc;
                EnemyWingedSkeletonCreate(head);
                head->unk_4FC.unk_4FC_entity = entity;
            }
        }
        return;
    case 4:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
