/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   GhostDancer_UpdateBestiaryPreview = Imp_UpdateState4Animation
 *   GhostDancer_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code/code_080211F0.h"

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Bestiary state (5): start animation 0 once, then update it.
 */
void GhostDancer_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Apply stored contact damage with zero effect type and flags at
 * the entity position.
 */
void GhostDancer_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
