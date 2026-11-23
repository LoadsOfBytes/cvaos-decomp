#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, u8);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

extern void Zombie_SpawnPiece(struct EwramData_EntityData *, s32, s32);

/* This inline step preserves the original order of the coordinate additions.
 * Combining it with the position expression changes constant reuse and registers. */
static inline s32 Offset(s32 value, s32 delta)
{
    value += delta;
    return value;
}

/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x70, animation 3, state 1), throwing two pieces
 * (Zombie_SpawnPiece).
 */
void Zombie_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, attacker, hit, (s16)damage);
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
        PlaySong(0x70);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
        {
            s32 x = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            s32 y;
            s32 r;
            r = RandomNumberGenerator() & 0x3FFFF;
            x += entity->posX.whole;
            y = Offset(r, -0x1A0000) + entity->posY.whole;
            Zombie_SpawnPiece(entity, x, y);
            x = (RandomNumberGenerator() & 0x7FFFF) - 0x40000;
            r = RandomNumberGenerator() & 0x3FFFF;
            x += entity->posX.whole;
            y = Offset(r, -0x1A0000) + entity->posY.whole;
            Zombie_SpawnPiece(entity, x, y);
        }
    }
}
