/* MedusaHead_UpdateIdle and MedusaHead_DamagePlayerOnContact. */
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 93BCC | Sub-state 0: start the animation (Anim_StartIndexed with
 * 0, 0, 0) and advance the sub-state.
 */
void MedusaHead_UpdateIdle(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->subState++;
    }
}

/**
 * @brief 93BF0 | Player_ApplyContactDamage with (attack, 0x100 when bit 0 of spawnParam1
 * is set else 0, 0, 1) at the entity position.
 */
void MedusaHead_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    if (entity->spawnParam1 & 1)
        contactDamage.attributes = 0x100;
    else
        contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
