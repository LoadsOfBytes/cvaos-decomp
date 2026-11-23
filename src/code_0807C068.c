/* Report a hit effect and signal the parent that contact occurred. */
#include "code/code_080211F0.h"
/**
 * @brief Contact (body and attack box): Player_ApplyContactDamage(attack, 0, 0, 0)
 * and set ext.persephone.contactHit on the entity in ENTITY_OWNER (the owner of an attack
 * box).
 */
void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    (ENTITY_OWNER(entity))->ext.persephone.contactHit = 1;
}
