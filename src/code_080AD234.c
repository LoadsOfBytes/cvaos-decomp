/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BigGolem_DamagePlayerOnContact = Enemy_DamagePlayerOnContact
 *   BigGolem_OnHit = Manticore_OnHit */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

extern void BigGolem_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief AD234 | Same as Enemy_DamagePlayerOnContact.
 */
void BigGolem_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief AD260 | Same as Manticore_OnHit.
 */
void BigGolem_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    BigGolem_OnDamaged(entity, attacker, hit, damage);
}
