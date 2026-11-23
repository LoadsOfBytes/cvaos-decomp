/* Lilith_DamagePlayerOnContact. */
#include "code/code_080211F0.h"
#include "lilith_entity.h"
#include "structs/ewram.h"

/**
 * @brief 952EC | Set LILITH_LUNGE_HIT and Player_ApplyContactDamage with (attack, 0, 0, 1)
 * at the entity position.
 */
void Lilith_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    LILITH_LUNGE_HIT(entity) = 1;
    Player_ApplyContactDamage(&contactDamage);
}
