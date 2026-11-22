#include "code_08009A0.h"
#include "structs/ewram.h"
/**
 * @brief Rise and blink near expiry; kind 0 deletes at tick 28 and kinds 1/2 at tick 40.
 */
void EntityDamageIndicator_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->posY.whole -= 0x8000;
        if (entity->timer > 20)
        {
            if (entity->timer % 2) entity->hidden = 1;
            else entity->hidden = 0;
        }
        if (++entity->timer == 28)
            entity->deleteRequested = 1;
        break;
    case 1:
    case 2:
        entity->posY.whole -= 0x8000;
        if (entity->timer > 30)
        {
            if (entity->timer % 2) entity->hidden = 1;
            else entity->hidden = 0;
        }
        if (++entity->timer == 40)
            entity->deleteRequested = 1;
        break;
    }
}
