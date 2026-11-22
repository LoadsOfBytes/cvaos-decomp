/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   DiscArmorDisc_DamagePlayerOnContact = MinotaurAxe_DamagePlayerOnContact */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief B240C | Same as MinotaurAxe_DamagePlayerOnContact.
 */
void DiscArmorDisc_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unk_4 = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
