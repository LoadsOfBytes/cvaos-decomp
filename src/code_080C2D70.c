/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ripper_OnHit = Fleaman_OnHit */
#include "structs/ewram.h"

extern void Ripper_UpdateDeath(struct EwramData_EntityData *entity);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief Resolve and display damage with hit flash; subtract signed damage
 * from HP and begin the death helper immediately on defeat.
 */
void Ripper_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        entity->subState = 0;
        Ripper_UpdateDeath(entity);
    }
}
