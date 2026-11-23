/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Dryad_DamagePlayerOnContact = Minotaur_DamagePlayerOnContact
 *   DryadSeed_OnHit = EventScene21_Init */
#include "code/code_080211F0.h"
#include "structs/ewram.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"

/**
 * @brief AEB50 | Same as Minotaur_DamagePlayerOnContact.
 */
void Dryad_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief AEB80 | Same as EventScene21_Init.
 */
void DryadSeed_OnHit(struct EwramData_EntityData *entity)
{
    entity->deleteRequested = 1;
}
