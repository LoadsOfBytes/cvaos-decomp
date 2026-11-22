/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GladiatorShotTip_DamagePlayerOnContact = GreatArmorShockwave_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief A1B50 | Same as GreatArmorShockwave_DamagePlayerOnContact.
 */
void GladiatorShotTip_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
