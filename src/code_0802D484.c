/* Use the selected target, or the nearest of the four registered candidates. */
#include "structs/ewram.h"

/**
 * @brief Return the selected target when non-NULL, otherwise the nearest of four non-NULL candidates; retain first-entry tie behavior and NULL when empty.
 */
struct EwramData_EntityData *Player_GetSelectedOrNearestTarget(struct EwramData_EntityData *entity)
{
    u32 bestDistance = -1;
    struct EwramData_EntityData *best = NULL;
    struct EwramData_EntityData *selected = gEwramData->playerState.selectedTarget;
    if (selected == NULL)
    {
        s32 i;
        for (i = 0; i < 4; i++)
        {
            struct EwramData_EntityData *target = gEwramData->playerState.trackedEntities[i];
            if (target != NULL)
            {
                s32 dx = (s16)target->posX.part16.integer - (s16)entity->posX.part16.integer;
                s32 dy = target->posY.part16.integer - entity->posY.part16.integer;
                u32 distance = dx * dx + dy * dy;
                if (bestDistance > distance)
                {
                    bestDistance = distance;
                    best = target;
                }
            }
        }
        return best;
    }
    return selected;
}
