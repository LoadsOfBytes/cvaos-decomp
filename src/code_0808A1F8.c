/* Enter the looping display animation once the first animation completes. */
#include "code/code_08039340.h"
/**
 * @brief Bestiary preview state (state 2): after the first animation loop
 * animation 1.
 */
void Ghost_UpdatePreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0 && entity->animDone)
    {
        entity->subState = 1;
        Anim_StartIndexed(entity, 1, 0, 1);
    }
}
