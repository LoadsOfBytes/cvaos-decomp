/* Advance the display sequence through three animations. */
#include "code/code_08039340.h"
/**
 * @brief Bestiary preview state (state 3): every 60 frames open (animations 1
 * and 3) and close again.
 */
void AluraUne_UpdatePreview(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    switch (state)
    {
    case 0:
        if (entity->timer++ > 59)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 2:
        if (entity->animDone)
        {
            entity->subState = 3;
            Anim_StartIndexed(entity, 4, 0, 1);
        }
        break;
    }
}
