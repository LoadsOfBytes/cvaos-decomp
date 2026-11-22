/* Delayed accelerated flight; retire outside the scene's bounds. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "code/code_08060B98.h"
/**
 * @brief After its delay, show the spark and move it with acceleration; delete
 * outside the box.
 */
void EventScene30Spark_Update(struct EwramData_EntityData *entity)
{
    if(entity->state==0)
    {
        if(--entity->timer==255){entity->hidden=0;entity->state++;}
    }
    else if(entity->state==1)
    {
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
        entity->velX.whole+=entity->accelX.whole;
        entity->velY.whole+=entity->accelY.whole;
        if(!Entity_IsInRect(entity,-128,-64,496,288))entity->deleteRequested=1;
    }
}
