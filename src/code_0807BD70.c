/* Alternate Persephone's two display animations after a timed pause. */
#include "code/code_08039340.h"
#include "code_08009A0.h"
/**
 * @brief Bestiary preview state (state 4): every 120 frames play animation 1 once,
 * then idle again.
 */
void Persephone_UpdatePreview(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    switch (state)
    {
    case 0:
        if (entity->timer++ > 119)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 0;
            {
                s32 timer = RandomNumberGenerator() & 31;
                entity->timer = timer;
            }
        }
        break;
    }
}
