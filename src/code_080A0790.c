/* KyomaDemon_UpdateDeath. */
#include "structs/ewram.h"

extern s32 Entity_GetVisibilityPattern(s32 step);

/**
 * @brief A0790 | Substate 0: blink while the timer timer runs, then
 * advance the substate.
 */
void KyomaDemon_UpdateDeath(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
            entity->subState++;
    }
}
