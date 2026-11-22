/* Update a Ghost or its timed spawn controller. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "structs/entity_template.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Ghost_UpdateFloat(struct EwramData_EntityData *);
extern void Ghost_UpdatePreview(struct EwramData_EntityData *);
extern void EnemyGhostCreate(struct EwramData_EntityData *);
void EnemyGhostUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (entity->state != 3)
    {
        if (entity->state != 1)
        {
            if (EntityIsOutsideScreen(entity, 4)) goto finished;
            if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        }
        if (EntityIsOutsideScreen(entity, 4))
        {
            s32 zero = 0;
            entity->state = 1;
            entity->timer = 1;
            entity->unk_518.part16.hp = zero;
        }
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 1) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity))
    {
        entity->velX.whole = 0;
        if (entity->state != 1)
        {
            Hit_CheckWithPlayer(collision, entity);
            goto finished;
        }
    }
    switch (entity->state)
    {
    case 0: Ghost_UpdateFloat(entity); break;
    case 1:
        Enemy_UpdateFragmentDeath(entity);
        if (entity->deleteRequested)
        {
            struct EwramData_EntityData *controller = entity->unk_4F8.unk_4F8_entity;
            if (controller && controller->updateFunc && controller->unk_518.part8.enemyId == 14
                && controller->unk_4FC.part16.unk_4FC != 0)
                controller->unk_4FC.part16.unk_4FC--;
        }
        break;
    case 2:
        Ghost_UpdatePreview(entity);
        Anim_Update(entity);
        goto finished;
    case 3:
    {
        s32 spawn = 0;
        if (entity->unk_4FC.part16.unk_4FE[0]++ >= entity->unk_514.part16.spawnParam1
            && (RandomNumberGenerator() & 31) == 0)
        {
            entity->unk_4FC.part16.unk_4FE[0] = spawn;
            spawn = 1;
        }
        if (spawn && entity->unk_4FC.part16.unk_4FC < entity->unk_514.part16.spawnParam0)
        {
            struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child)
            {
                s32 distance;
                entity->unk_4FC.part16.unk_4FC++;
                do
                {
                    child->posX.part16.integer = RandomNumberGenerator() % 240;
                    distance = gEwramData->playerState.player->posX.whole - child->posX.whole;
                    if (distance < 0) distance = -distance;
                } while (distance <= 0x300000);
                do
                {
                    child->posY.part16.integer = RandomNumberGenerator() % 176 + 16;
                    distance = gEwramData->playerState.player->posY.whole - child->posY.whole;
                    if (distance < 0) distance = -distance;
                } while (distance <= 0x300000);
                child->scrollWithBg = 1;
                child->unk_518.part8.spawnIndex = 0;
                child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
                child->unk_514.part16.spawnParam1 = 0;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                child->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
                child->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
                child->updateFunc = entity->updateFunc;
                EnemyGhostCreate(child);
                child->unk_4F8.unk_4F8_entity = entity;
            }
        }
        goto finished;
    }
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
