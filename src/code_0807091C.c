/* Animate a detached sprite for twelve ticks while moving upward. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void EntityMove(struct EwramData_EntityData *);
/**
 * @brief Move upward with alternating horizontal velocity, decrement the frame at tick 6 and delete after tick 11.
 */
void KillerFishEffect_UpdateRising(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        entity->state = 1;
        entity->timer = 0;
        /* fall through */
    case 1:
        entity->timer++;
        if ((entity->timer & 7) == 0)
        {
            if (entity->timer & 8) entity->velX.whole = -0x4000;
            else entity->velX.whole = 0x4000;
        }
        entity->velY.whole = -0x4000;
        EntityMove(entity);
        if (entity->timer == 6) entity->animFrame--;
        if (entity->timer > 11) EntityDelete(entity);
        break;
    }
}
