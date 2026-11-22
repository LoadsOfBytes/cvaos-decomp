/* Copies the three hit slots to the child entity, tests it and copies them back on a hit. */
#include "structs/ewram.h"
#include "entity_hit.h"

/**
 * @brief Copy three hit-history slots from owner to entity, check collision with the player and copy them back only when the check reports a hit.
 */
u16 Hit_CheckWithPlayerSharedHistory(struct HitResult *res, struct EwramData_EntityData *entity, struct EwramData_EntityData *owner)
{
    s32 i;
    u32 hit;

    for (i = 0; i <= 2; i++)
    {
        entity->hit.hitBy[i] = owner->hit.hitBy[i];
        entity->hit.hitTimer[i] = owner->hit.hitTimer[i];
    }
    hit = (u16)Hit_CheckWithPlayer(res, (u8 *)entity);
    if (hit != 0)
    {
        for (i = 0; i <= 2; i++)
        {
            owner->hit.hitBy[i] = entity->hit.hitBy[i];
            owner->hit.hitTimer[i] = entity->hit.hitTimer[i];
        }
    }
    return hit;
}
