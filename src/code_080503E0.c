#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
/**
 * @brief Rise slowly and delete at the end of the animation or off screen.
 */
void BreakableBlockDust_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->velY.whole = -0xC000 - ((RandomNumberGenerator() & 0xFF) << 4);
        entity->state++;
        break;
    case 1:
        entity->posY.whole += entity->velY.whole;
        break;
    }
    if (!Entity_IsInRect(entity, -16, -16, 272, 192)) entity->deleteRequested = 1;
    if (Anim_Update(entity) == 3) entity->deleteRequested = 1;
}
