/* Apply damage, choose death, or occasionally react to attack types 3 and 4. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "code_08009A0.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x1CC, 70 frames, state 2); a hit of type 3 / 4
 * knocks it down with 1/4 chance (animation 6, state 3).
 */
void Persephone_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 attack = Player_CalculateAttackPower(hit[2]);
    u8 *attackType = &attacker->hit.type;
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)attack);
    s32 zero;
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;
        damage = (s16)damage;
        zero = 0;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1CC);
        Anim_StartIndexed(entity, 7, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 70;
        entity->state = 2;
        entity->subState = zero;
    }
    else if ((u8)(*attackType - 3) <= 1 && (RandomNumberGenerator() & 255) <= 63)
    {
        Anim_StartIndexed(entity, 6, 0, 0);
        entity->state = 3;
        entity->subState = zero;
        entity->timer = zero;
    }
}
