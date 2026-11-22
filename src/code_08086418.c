/* Emit the contact effect, variant 1 when unk_4FC is 3. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Apply stored body contact damage with effect type 0 and
 * flags 1 only for phase 3; other phases use flags 0.
 */
void Headhunter_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unk_4 = 0;
    if (entity->unk_4FC.part8.unk_4FC != 3)
        effect.reaction = 0;
    else
        effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
}
