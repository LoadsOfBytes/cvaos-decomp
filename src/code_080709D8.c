/* Emit the part's configured effect at its current position. */
#include "code/code_080211F0.h"
/**
 * @brief Contact: mark the bite (unk_500 = 1) and
 * Player_ApplyContactDamage(attack, 0, 0, 1) at the fish position.
 */
void KillerFishSwarm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unk_4 = 0;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    entity->unk_500.whole = 1;
    Player_ApplyContactDamage(&effect);
}
