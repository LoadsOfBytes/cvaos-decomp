/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   KillerDoll_UpdateState2Animation = Ukoback_UpdateState1Animation
 *   KillerDoll_OnContact = Enemy_DamagePlayerOnContact */
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
 * @brief Start animation 0 once and then advance it. The gameplay purpose of state 2 remains unresolved.
 */
void KillerDoll_UpdateState2Animation(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Submit the attack contact record with mode fields 0, 0, 0.
 */
void KillerDoll_OnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
