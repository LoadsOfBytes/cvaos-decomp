/* GladiatorShot_OnHit. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

/**
 * @brief A1B3C | Hit callback: Player_CalculateAttackPower(hit[2]) and state 1.
 */
void GladiatorShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 1;
}
