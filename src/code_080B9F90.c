/* WitchSpell_DamagePlayerOnContact. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief Apply contact damage equal to the stored damage plus one quarter,
 * with effect type 2 and zero effect flags at the spell position.
 */
void WitchSpell_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack + (entity->attack >> 2);
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
