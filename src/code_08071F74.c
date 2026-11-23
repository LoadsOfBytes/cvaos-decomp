/* Apply Mudman damage and choose death or a hit-dependent reaction state. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
/**
 * @brief Hit: resolve, flash, subtract and show the damage; at HP <= 0 drop the
 * rewards and die (song 0x70, animation 4, state 2); a hit of type 3 or 4
 * starts the hurt state 3.
 */
void Mudman_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 power = Player_CalculateAttackPower(hit[2]);
    u8 *value554 = &attacker->hit.type;
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)power);
    s32 zero;
    entity->hitFlashTimer = 8;
    {
        s32 hp = entity->hp;

        zero = 0;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x70);
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->animDone = 0;
        entity->palette = entity->basePalette;
        entity->timer = 40;
        entity->state = 2;
        entity->subState = zero;
    }
    else if ((u8)(*value554 - 3) <= 1)
    {
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->animDone = 0;
        entity->state = 3;
        entity->subState = zero;
    }
}

