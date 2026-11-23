/* Stolas hit callback: on death, emit six projectiles with random velocities. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void Stolas_SpawnDebris(struct EwramData_EntityData *, s32, s32, s32, s32);
void Stolas_OnDamaged(struct EwramData_EntityData *entity, void *attacker, void *hit, s32 damage)
{
    entity->hitFlashTimer = 8;
    entity->hp -= damage;
    if ((s16)entity->hp <= 0)
    {
        s32 i;
        u32 range;
        s32 offset;
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 3, 0, 1);
        PlaySong(0x6E);
        entity->palette = entity->basePalette;
        entity->timer = 60;
        entity->state = 2;
        entity->subState = 0;
        range = 0x60000;
        offset = -0x30000;
        for (i = 5; i >= 0; i--)
        {
            s32 vx = RandomNumberGenerator() % range + offset;
            s32 vy = RandomNumberGenerator() % range + offset;
            Stolas_SpawnDebris(entity, 0, -0x1D0000, vx, vy);
        }
    }
}
