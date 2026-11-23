/* Report contact effects and apply the parent's selected knockback response. */
#include "code/code_080211F0.h"
#include "joint_entity.h"
#include "golem_entity.h"
/**
 * @brief Contact callback of a golem segment: Player_ApplyContactDamage with the
 * segment's attack attack at its position; reaction 1, or in the parent's
 * attack mode unk_4FC == 1 reaction 3 with a knockback velocity. The call
 * after the mode switch applies the damage again in every mode (kept).
 */
void GolemSegment_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = JOINT_PART_OWNER(entity);
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unused = 0;
    effect.reaction = 1;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    switch (GOLEM_ATTACK_PHASE(parent))
    {
    case 0:
        effect.reaction = 1;
        Player_ApplyContactDamage(&effect);
        break;
    case 1:
    {
        s32 vx;
        effect.reaction = 3;
        Player_ApplyContactDamage(&effect);
        vx = -0x30000;
        if (gEwramData->playerState.player->posX.whole > entity->posX.whole) vx = -vx;
        Player_SetKnockbackVelocity(vx, -0x60000);
        break;
    }
    }
    Player_ApplyContactDamage(&effect);
}
