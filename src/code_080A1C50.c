/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Gladiator_DamagePlayerOnContact = Object28_OnPlayerContact
 *   Gladiator_OnHit = Manticore_OnHit
 *   GladiatorPart_DamagePlayerOnContact = Object28_OnPlayerContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

extern void Gladiator_OnDamaged(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s32 damage);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief A1C50 | Same as Object28_OnPlayerContact.
 */
void Gladiator_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief A1C80 | Same as Manticore_OnHit.
 */
void Gladiator_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Gladiator_OnDamaged(entity, attacker, hit, damage);
}

/**
 * @brief A1CC8 | Same as Object28_OnPlayerContact.
 */
void GladiatorPart_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
