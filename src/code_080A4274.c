/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ChaosSeedShot_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 param_2, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unk_4 = param_2;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief A4274 | Same as Enemy_DamagePlayerOnContact.
 */
void ChaosSeedShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* A42A0 | Contact callback of the sparks: mark the grab (unk_500 = 1);
 * ChaosSpark_Update then holds the player until they break free. */
void ChaosSpark_OnContact(struct EwramData_EntityData *entity)
{
    entity->unk_500.part8.unk_500 = 1;
}
