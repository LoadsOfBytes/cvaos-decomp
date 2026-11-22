/* Clear a matching id and its timer from an entity's three box slots. */
#include "structs/ewram.h"

/**
 * @brief Unreferenced: clear the hit record (attacker and timer) of the given
 * attacker in the entity's three slots; return 1 when found.
 */
s32 Hit_ClearHitBy(struct EwramData_EntityData *entity, s32 attacker)
{
    struct EntityHit *hit = &entity->hit;
    if (hit->hitBy[0] == attacker)
    {
        hit->hitBy[0] = 0;
        hit->hitTimer[0] = 0;
    }
    else if (hit->hitBy[1] == attacker)
    {
        hit->hitBy[1] = 0;
        hit->hitTimer[1] = 0;
    }
    else if (hit->hitBy[2] == attacker)
    {
        hit->hitBy[2] = 0;
        hit->hitTimer[2] = 0;
    }
    else
        return 0;
    return 1;
}
