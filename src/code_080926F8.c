#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void ZombieSoldier_UpdateWalk(struct EwramData_EntityData *entity);
extern void ZombieSoldier_UpdateAttack(struct EwramData_EntityData *entity);
extern void ZombieSoldier_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame);

/**
 * @brief 926F8 | Zombie Soldier update: skipped by Enemy_PreUpdate; states 0
 * / 1 (ZombieSoldier_UpdateWalk / ZombieSoldier_UpdateAttack), 2 dying: on frame 0x28 two random
 * body parts ZombieSoldier_SpawnPiece (frames 0x13 / 0x14, velX away from the facing),
 * 3 animation only. The 0x30-byte stack buffer is unused in the original.
 */
void EnemyZombieSoldierUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[0x30];
    s32 x, y, vx;

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
    case 0:
        ZombieSoldier_UpdateWalk(entity);
        break;
    case 1:
        ZombieSoldier_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer == 0x28)
        {
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -0x10000 - (RandomNumberGenerator() & 0xFFFFF);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieSoldier_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x13);
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -(RandomNumberGenerator() % 0x180000);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieSoldier_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x14);
        }
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}
