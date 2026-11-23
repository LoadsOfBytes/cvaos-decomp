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
 * its spawner (unk_4FC, enemyId 0x14); the spawner (state 3) with 1/32
 * chance creates one below the limit spawnParam0: x off the room edge farther
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
        if ((s16)entity->hp <= 0)
            WingedSkeleton_SpawnDeathPieces(entity);
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            parent = entity->ext.wingedSkeleton.spawner;
            if (parent != NULL && parent->updateFunc != NULL && parent->enemyId == 0x14
                && parent->ext.wingedSkeletonSpawner.childCount != 0)
                parent->ext.wingedSkeletonSpawner.childCount--;
        }
        break;
    case 3:
        if ((RandomNumberGenerator() & 0x1F) == 0 && (s32)entity->ext.wingedSkeletonSpawner.childCount < entity->spawnParam0)
        {
            head = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (head != NULL)
            {
                entity->ext.wingedSkeletonSpawner.childCount++;
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
                head->spawnIndex = 0;
                head->enemyId = entity->enemyId;
                head->spawnParam1 = 0;
                head->drawFunc = (u32 *)Sprite_DrawEntity;
                head->hp = sEnemyData[entity->enemyId].hp;
                head->mp = sEnemyData[entity->enemyId].mp;
                head->updateFunc = entity->updateFunc;
                EnemyWingedSkeletonCreate(head);
                head->ext.wingedSkeleton.spawner = entity;
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
