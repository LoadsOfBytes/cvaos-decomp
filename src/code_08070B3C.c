/* Dispatch Merman states, including the hidden instance's child spawning. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/entity_template.h"
extern void EnemyMermanCreate(struct EwramData_EntityData *);
extern void Merman_UpdateMove(struct EwramData_EntityData *);
extern void Merman_UpdateAttack(struct EwramData_EntityData *);
extern s32 Entity_IsOutsideRoomMap(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_MoveWithOptionalCeilingCollision(struct EwramData_EntityData *, s32, s32, s32);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
void EnemyMermanUpdate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    u8 collision[16];
    if (entity->state != 3)
    {
        if (entity->state != 2 && Entity_IsInRect(entity, -16, 0, 272, 176))
            Player_TrackEntity(entity);
        if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
        else entity->hidden = 0;
        if (Entity_IsOutsideRoomMap(entity))
        {
            entity->state = 2;
            entity->timer = 1;
        }
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 2) Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0: Merman_UpdateMove(entity); break;
    case 1: Merman_UpdateAttack(entity); break;
    case 2:
        {
            struct EwramData_EntityData *parent;
            Entity_MoveWithOptionalCeilingCollision(entity, 0x20000, -0x200000, 0);
            Enemy_UpdateFragmentDeath(entity);
            if (entity->deleteRequested)
            {
                parent = entity->ext.merman.spawner;
                if (parent && parent->updateFunc && parent->unk_518.part8.enemyId == 3)
                {
                    if (parent->ext.mermanSpawner.childCount) parent->ext.mermanSpawner.childCount--;
                }
            }
        }
        break;
    case 3:
        {
            s32 zero = RandomNumberGenerator() & 7;
            struct EwramData_EntityData *child;
            s32 mode;
            if (zero != 0) goto finished;
            if ((s32)entity->ext.mermanSpawner.childCount > 2) goto finished;
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child == NULL) goto finished;
            mode = entity->unk_514.part16.spawnParam0;
            if (mode != 1) goto finished;
            entity->ext.mermanSpawner.childCount++;
            child->posX.whole = (((RandomNumberGenerator() & 0xFF) + 144) << 16) - background->xPos.whole;
            child->posY.whole = ((entity->ext.merman.homeY + 32) << 16) - background->yPos.whole;
            child->scrollWithBg = 1;
            child->unk_518.part8.spawnIndex = zero;
            child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
            child->unk_514.part16.spawnParam0 = mode;
            child->unk_514.part16.spawnParam1 = zero;
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            child->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
            child->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
            child->updateFunc = entity->updateFunc;
            EnemyMermanCreate(child);
            entity->timer = 60;
            child->ext.merman.spawner = entity;
            child->ext.merman.homeX = entity->ext.merman.homeX;
            child->ext.merman.homeY = entity->ext.merman.homeY;
        }
        goto finished;
    case 4: Anim_Update(entity); goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
