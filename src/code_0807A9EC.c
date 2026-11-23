/* Update a Zombie or its controller that maintains a population of children. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "structs/entity_template.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Zombie_UpdateEmergingWalkingSinking(struct EwramData_EntityData *);
extern void Zombie_UpdateStationaryAnimation(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void EnemyZombieCreate(struct EwramData_EntityData *);
void EnemyZombieUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 3)
    {
        if (entity->state != 1 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4))
        {
            s32 zero = 0;
            entity->state = 1;
            entity->timer = 1;
            entity->unk_518.part16.hp = zero;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 1) Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0:
        Zombie_UpdateEmergingWalkingSinking(entity);
        break;
    case 1:
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            struct EwramData_EntityData *controller = entity->ext.zombie.spawner;
            if (controller && controller->updateFunc && controller->unk_518.part8.enemyId == 1
                && controller->ext.zombieSpawner.childCount != 0)
                controller->ext.zombieSpawner.childCount--;
        }
        break;
    case 2:
        Zombie_UpdateStationaryAnimation(entity);
        Anim_Update(entity);
        goto finished;
    case 3:
        if ((RandomNumberGenerator() & 31) == 0
            && (s32)entity->ext.zombieSpawner.childCount < entity->unk_514.part16.spawnParam0)
        {
            struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child)
            {
                s32 distance;
                entity->ext.zombieSpawner.childCount++;
                do
                {
                    child->posX.part16.integer = RandomNumberGenerator() % 240;
                    distance = gEwramData->playerState.player->posX.whole - child->posX.whole;
                    if (distance < 0) distance = -distance;
                } while (distance <= 0x200000);
                child->posY.whole = entity->posY.whole;
                child->scrollWithBg = 1;
                child->unk_518.part8.spawnIndex = 0;
                child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
                child->unk_514.part16.spawnParam1 = 0;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                child->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
                child->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
                child->updateFunc = entity->updateFunc;
                EnemyZombieCreate(child);
                child->ext.zombie.spawner = entity;
            }
        }
        goto finished;
    }
    {
        u8 *frame = &entity->animFrame;
        u8 previous = *frame;
        Anim_Update(entity);
        if (previous != *frame) Entity_UpdateBoxes(entity);
    }
    if (entity->state == 0 && (entity->subState == 0 || entity->subState == 3))
        entity->hit.flags |= 2;
    else
        entity->hit.flags &= ~2;
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
