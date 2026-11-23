/* Hit-effect strength follows the parent's current action phase. */
#include "code/code_080211F0.h"
#include "werewolf_entity.h"

/**
 * @brief Contact: Player_ApplyContactDamage with half the attack, the full attack
 * and knockback mode 1 during the dash (state 1, sub-states 1..2; WEREWOLF_DASH_HIT
 * records it).
 */
void Werewolf_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attributes = 0;
    effect.unused = 0;
    effect.attack = entity->attack >> 1;
    effect.reaction = 0;
    WEREWOLF_DASH_HIT(entity) = 0;
    if (entity->state == 1 && (u8)(entity->subState - 1) <= 1)
    {
        effect.attack = entity->attack;
        effect.reaction = 1;
        WEREWOLF_DASH_HIT(entity) = 1;
    }
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}
