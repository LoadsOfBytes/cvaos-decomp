/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BomberArmorBomb_DamagePlayerOnContact = HeadStackProjectile_DamagePlayerOnContact
 *   RockArmorRock_DamagePlayerOnContact = DeathPillar_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief B2F64 | Same as HeadStackProjectile_DamagePlayerOnContact.
 */
void BomberArmorBomb_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B2F94 | Same as DeathPillar_DamagePlayerOnContact.
 */
void RockArmorRock_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
