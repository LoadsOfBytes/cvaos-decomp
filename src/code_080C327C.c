/* Play the defeated animation with the entity's palette and delete at its end. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
/**
 * @brief Death state (3): play animation 2 with the restored palette and
 * request deletion when the animation is done.
 */
void GhostDancer_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity,2,0,0);
            entity->palette=entity->basePalette;
            entity->objMode=1;
            entity->state=3;
            entity->subState++;
            entity->objMode=1;
        case 1:
            if(entity->animDone)
                entity->deleteRequested=1;
            break;
    }
}
