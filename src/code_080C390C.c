/* Skeleton_UpdateIdle. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Start animation 0 and count down 60 ticks before returning to combat movement state 1.
 */
void Skeleton_UpdateIdle(struct EwramData_EntityData *entity)
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
