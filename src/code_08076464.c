/* HarpyFeather_OnHit. */
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);

/**
 * @brief 76464 | Hit: call Player_CalculateAttackPower on the damage value (result
 * unused) and go to state 1.
 */
void HarpyFeather_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
    entity->subState = 0;
}
