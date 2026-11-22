/* Object0E_OnHit. */
#include "structs/ewram.h"

/**
 * @brief 50D98 | velX = 1, or -1 when entity is left of attacker.
 */
void Object0E_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker)
{
    s32 speed = 1;

    entity->velX.whole = speed;
    if (entity->posX.whole < attacker->posX.whole)
        entity->velX.whole = -speed;
}
