/* Lilith_DamagePlayerOnContact. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief 952EC | Set unk_4FE = 1 and Player_ApplyContactDamage with (attack, 0, 0, 1)
 * at the entity position.
 */
void Lilith_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    entity->unk_4FC.part8.unk_4FE = 1;
    Player_ApplyContactDamage(&contactDamage);
}
