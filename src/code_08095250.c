/* LilithShot_DamagePlayerOnContact. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief 95250 | Player_ApplyContactDamage with (attack, 0x80, 0, 0) and state 2.
 */
void LilithShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x80;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 2;
    entity->subState = 0;
}
