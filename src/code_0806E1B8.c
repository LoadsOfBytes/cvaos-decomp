/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Enemy_OnContact = Minotaur_DamagePlayerOnContact
 *   Enemy_OnContactMask1 = Entity_OnPlayerContactDamage */
#include "code/code_080211F0.h"
#include "structs/ewram.h"
#include "gba/dma.h"
#include "code_08009A0.h"
#include "code/code_08039340.h"

/**
 * @brief Apply player contact damage from the entity attack value and position with effect and reaction fields zero.
 */
void Enemy_OnContact(struct EwramData_EntityData *entity)
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
 * @brief Apply player contact damage from the entity attack value and position with effect mask 1 and reaction field zero.
 */
void Enemy_OnContactMask1(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
