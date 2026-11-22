#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Local 0x24-byte template-table view: retain this TU's types and padding. */
struct EnemyTemplateView
{
    u8 pad_0[0xC];
    u16 hp;
    u16 unk_E;
    u8 pad_10[0x24 - 0x10];
};

extern const struct EnemyTemplateView sEnemyData[];
extern s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern void EnemyUneCreate(struct EwramData_EntityData *entity);
extern void EntityUpdateNothing(void);
extern void Une_SpawnDebris(s32, s32, s32, s32, s32);

/**
 * @brief 743B4 | Une spawner AI: falls/bounces until it lands (Entity_MoveWithPointCollision),
 * wakes on the player box 0xC8 x 0x140 (state 10 = spawned child, straight
 * to state 1), opens (animations 1-3) and waits; when the timer unk_504
 * runs out it checks the parent enemy 0x3D's child count (unk_4FC vs
 * unk_514) and either keeps spawning (state 4: a child Une every 30 frames
 * via EnemyUneCreate, with debris Une_SpawnDebris) or closes (animation 5,
 * then state = 2).
 */
void Une_UpdateGrow(struct EwramData_EntityData *entity)
{
    s32 res;
    s32 speed;
    struct EwramData_EntityData *parent;
    struct EwramData_EntityData *child;
    u8 t;
    s32 vx;
    s32 vy;

    switch (entity->subState)
    {
    case 0:
    case 10:
        if (entity->accelY.whole)
        {
            speed = 0x10000;
            res = Entity_MoveWithPointCollision(entity, speed, -0x80000);
            if (res & 3)
                entity->velX.whole = -entity->velX.whole;
            if (res & 8)
                entity->velY.whole += speed;
        }
        else
        {
            res = 0xF;
        }
        if (!(res & 4))
            break;
        entity->velX.whole = 0;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0;
        if (entity->subState == 10)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->animDone = 0;
        }
        else if (Target_IsInRect(entity->posX.part16.integer - 0x64, entity->posY.part16.integer - 0xA0, 0xC8, 0x140))
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState++;
            entity->animDone = 0;
        }
        entity->unk_504.whole = RandomNumberGenerator() % 200 + 0x118;
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState++;
            entity->animDone = 0;
        }
        break;
    case 2:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->subState++;
            entity->animDone = 0;
            entity->timer = RandomNumberGenerator() % 40 + 100;
            if (entity->unk_500.whole)
                entity->timer = 10;
        }
        break;
    case 3:
        t = entity->timer--;
        if (t == 0)
        {
            parent = entity->unk_4FC.unk_4FC_entity;
            if (parent != NULL && parent->updateFunc != NULL && parent->unk_518.part8.enemyId == 0x3D
                && (s32)parent->unk_4FC.whole < parent->unk_514.part16.spawnParam0)
            {
                entity->timer = t;
                entity->subState++;
                break;
            }
            entity->timer = RandomNumberGenerator() % 40 + 100;
        }
        if (--entity->unk_504.whole > 0)
            break;
        parent = entity->unk_4FC.unk_4FC_entity;
        if (parent == NULL || parent->updateFunc == NULL || parent->unk_518.part8.enemyId != 0x3D)
            break;
        if ((s32)parent->unk_4FC.whole >= parent->unk_514.part16.spawnParam0)
        {
            entity->subState = 5;
            Anim_StartIndexed(entity, 5, 0, 0);
            entity->animDone = 0;
        }
        else
        {
            entity->unk_504.whole = 0x8C;
        }
        break;
    case 4:
        if (entity->timer++ > 0x1D)
        {
            entity->timer = RandomNumberGenerator() % 40 + 100;
            entity->subState = 3;
        }
        if (entity->timer == 0xF)
        {
            parent = entity->unk_4FC.unk_4FC_entity;
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child != NULL)
            {
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole - 0x10000;
                child->scrollWithBg = 1;
                child->unk_518.part8.spawnIndex = 0;
                child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
                child->unk_514.part16.spawnParam0 = 0;
                child->unk_514.part16.spawnParam1 = 0;
                child->drawFunc = (u32 *)Sprite_DrawEntity;
                child->unk_518.part16.hp = sEnemyData[entity->unk_518.part8.enemyId].hp;
                child->unk_510.part16.unk_512 = sEnemyData[entity->unk_518.part8.enemyId].unk_E;
                child->updateFunc = entity->updateFunc;
                EnemyUneCreate(child);
                child->velX.whole = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
                child->velY.whole = -0x30000 - (RandomNumberGenerator() & 0x1FFFF);
                child->accelX.whole = 0;
                child->accelY.whole = 0x2800;
                child->subState = 10;
                child->unk_4FC.unk_4FC_entity = parent;
                parent->unk_4FC.whole++;
                child->unk_500.whole = 0;
            }
            else
            {
                parent = entity->unk_4FC.unk_4FC_entity;
                if (parent != NULL && parent->updateFunc != NULL && parent->unk_518.part8.enemyId == 0x3D)
                    parent->unk_4FC.whole--;
            }
        }
        if (!(entity->timer & 3))
        {
            vx = (RandomNumberGenerator() & 0x3FFFF) - 0x20000;
            vy = -0x20000 - (RandomNumberGenerator() & 0x1FFFF);
            Une_SpawnDebris(entity->posX.whole, entity->posY.whole, vx, vy, entity->spriteLayer - 1);
        }
        break;
    case 5:
        if (entity->animDone)
        {
            entity->state = 2;
            entity->timer = 1;
        }
        break;
    }
}
