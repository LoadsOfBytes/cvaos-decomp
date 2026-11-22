/* RipperProjectile_DamagePlayerOnContact. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief Apply stored contact damage with zero effect type/flags, set hit
 * flash and hit flags 2/4, then enter timed removal state 6.
 */
void RipperProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->unk_510.part8.hitFlashTimer = 8;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->state = 6;
    entity->stage = 0;
    entity->subState = 0;
}
