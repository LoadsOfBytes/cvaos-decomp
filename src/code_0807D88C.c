#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "code_08009A0.h"

extern u16 Player_CalculateAttackPower(u16);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void GiantGhost_SpawnFragment(s32 x, s32 y, s32 vx, s32 vy, s32 layer, s32 flip);

/**
 * @brief 7D88C | Hit callback: flash (hitFlashTimer = 8), subtract the damage
 * (Hit_ResolveDamage) from the HP in hp and show it (Entity_ShowDamage); at 0 HP
 * release the hit box, play song 0x6B, animation 5, palette from basePalette,
 * death state 2 (timer 0x5A) and burst into four random fragments
 * (GiantGhost_SpawnFragment) one layer below.
 */
void GiantGhost_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    s32 i;
    damage = Hit_ResolveDamage(entity, (s32)other, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6B);
        Anim_StartIndexed(entity, 5, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x5A;
        entity->state = 2;
        entity->subState = 0;
        entity->objMode = 1;
        for (i = 0; i < 4; i++)
        {
            s32 dx = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            s32 dy = -(RandomNumberGenerator() & 0xFFFFF);
            s32 vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 vy = -(RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 flip = RandomNumberGenerator() & 1;
            GiantGhost_SpawnFragment(entity->posX.whole + dx, entity->posY.whole + dy, vx, vy, entity->spriteLayer - 1, flip);
        }
    }
}
