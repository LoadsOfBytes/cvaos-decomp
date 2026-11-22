/* PeepingEye_UpdateIdle. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 88E54 | Sub-state 0: start the animation (Anim_StartIndexed with
 * 0, 0, 0) and advance the sub-state.
 */
void PeepingEye_UpdateIdle(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 0);
        entity->subState++;
    }
}
