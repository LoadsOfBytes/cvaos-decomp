/* Move the red-soul effect and release its slot when it expires. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
/**
 * @brief Move with acceleration and delete the effect when it leaves the
 * screen.
 */
void BoneSoulEffect_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMoveX(entity, 0x80000);
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (EntityIsOutsideScreen(entity, 4)) entity->state = 1;
    if (--entity->timer == 0) entity->state = 1;
    if (!entity->roomActive) entity->state = 1;
    if (entity->state != 0)
    {
        gEwramData->playerState.activeRedSoulEntities[entity->ext.redSoul.slot] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
    }
    else Anim_Update(entity);
}
