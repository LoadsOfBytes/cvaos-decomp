/* Report a hit effect and signal the parent that contact occurred. */
#include "code/code_080211F0.h"
/**
 * @brief Contact (body and attack box): Player_ApplyContactDamage(attack, 0, 0, 0)
 * and set unk_504 = 1 on the entity in unk_4F8 (the owner of an attack
 * box).
 */
void Persephone_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unk_4 = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    entity->unk_4F8.unk_4F8_entity->unk_504.part8.unk_504 = 1;
}
