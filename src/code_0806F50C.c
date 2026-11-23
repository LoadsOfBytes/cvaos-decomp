/* Werewolf hit: HP, death state, and interrupting a close attack. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"

extern u16 Player_CalculateAttackPower(u16);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s16, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);

/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x6E, animation 4, state 2); a hit of type 3 / 4
 * during the dash makes it recoil (state 3).
 */
void Werewolf_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hitData)
{
    s32 damage = Player_CalculateAttackPower(hitData[2]);
    struct EntityHit *hit = &attacker->hit;
    damage = Hit_ResolveDamage(entity, (s32)attacker, hitData, (s16)damage);
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
        PlaySong(0x6E);
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
    else if (entity->state == 1 && (u8)(entity->subState - 1) <= 1
             && (u8)(hit->type - 3) <= 1)
    {
        Anim_StartIndexed(entity, 4, 0, 1);
        entity->state = 3;
        entity->subState = 0;
    }
}
