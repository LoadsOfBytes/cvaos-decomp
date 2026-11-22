/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BeamSkeleton_UpdateIdle = Skeleton_UpdateIdle */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Idle state (2): play animation 0 for a 60-update countdown, then
 * return to approach state 1.
 */
void BeamSkeleton_UpdateIdle(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex)
                Anim_StartIndexed(entity, 0, 0, 1);
            entity->timer = 0x3C;
            entity->subState++;
        case 1:
            if (--entity->timer == 0)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
}
