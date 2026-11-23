/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object18_OnHit = Object0E_OnHit
 *
 * Object19_OnPlayerContact decompiled locally. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief Hit callback of object 0x18: set velX to +1/-1 away from the attacker.
 */
void Object18_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    s32 speed = 1;

    entity->velX.whole = speed;
    if (entity->posX.whole < attacker->posX.whole)
        entity->velX.whole = -speed;
}

/**
 * @brief Player contact callback of object 0x19: contact damage of 2/3 of the
 * strength-based value used by Object21Create, at the entity position.
 */
void Object19_OnPlayerContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 power;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    power = (gEwramData->playerState.equipmentDefenseBonus >> 1) + (gEwramData->playerState.equipmentDefenseBonus >> 2);
    power += gEwramData->playerStatus.maxHP >> 4;
    power = (u16)power;
    contactDamage.attack = power * 2 / 3;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
