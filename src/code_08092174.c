#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void ZombieOfficer_UpdateWalk(struct EwramData_EntityData *entity);
extern void ZombieOfficer_UpdateAttack(struct EwramData_EntityData *entity);
extern void ZombieOfficer_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame);

/**
 * @brief 92174 | Zombie Officer update (as EnemyZombieSoldierUpdate):
 * states 0 / 1 (ZombieOfficer_UpdateWalk / ZombieOfficer_UpdateAttack), 2 dying: on frame 0x28 three
 * body parts ZombieOfficer_SpawnPiece (frames 0x10, 0x11 random, 0xF the head 6 px in
 * front and 13 px up), 3 animation only.
 */
void EnemyZombieOfficerUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    s32 x, y, vx;

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
    case 0:
        ZombieOfficer_UpdateWalk(entity);
        break;
    case 1:
        ZombieOfficer_UpdateAttack(entity);
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
            ZombieOfficer_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x10);
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -(RandomNumberGenerator() % 0x180000);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieOfficer_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x11);
            x = entity->flipX ? 0x60000 : -0x60000;
            y = -0xD0000;
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieOfficer_SpawnPiece(entity, x, y, vx, -0x10000 - (RandomNumberGenerator() & 0xFFFF), 0xF);
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
