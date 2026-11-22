#include "structs/ewram.h"
/* Start a half-pixel step, decelerate through zero, wait, then repeat. */
void Worm_StepCrawl(struct EwramData_EntityData *entity, s32 direction, s32 distance, s32 blocked)
{
    switch (entity->subState)
    {
    case 0:
        if (blocked == 0 || distance > 7)
        {
            if (direction != 0)
            {
                entity->velX.whole = 0x8000;
                entity->accelX.whole = -0x300;
            }
            else
            {
                entity->velX.whole = -0x8000;
                entity->accelX.whole = 0x300;
            }
            entity->subState = 1;
            entity->timer = 0;
        }
        break;
    case 1:
        entity->timer++;
        entity->velX.whole += entity->accelX.whole;
        if (entity->accelX.whole > 0)
        {
            if (entity->velX.whole >= 0)
                entity->subState = 2;
        }
        else if (entity->velX.whole <= 0)
            entity->subState = 2;
        break;
    case 2:
        if (entity->timer++ > 16)
        {
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
}
