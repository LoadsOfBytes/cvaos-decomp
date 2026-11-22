/* Sequence ten timed effects with a small independent positional jitter. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include <string.h>
struct EffectStep { u16 x,y; u8 delay; u8 padding[3]; };
extern struct EffectStep sEventScene30EmitterBSteps[];
extern void EventScene30_SpawnSparkBurstSmall(s16,s16);
/**
 * @brief Emitter callback 1: step through sEventScene30EmitterBSteps spawning small spark
 * bursts.
 */
void EventScene30EmitterB_Update(struct EwramData_EntityData *entity)
{
    struct EffectStep steps[10];
    s32 stage;
    memcpy(steps,sEventScene30EmitterBSteps,sizeof(steps));
    stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        u32 random=RandomNumberGenerator();
        s32 x,y,mask;
        struct EffectStep *step;
        u16 *coordinate;
        step=&steps[entity->stage];
        coordinate=&entity->posX.part16.integer;
        x=*coordinate;
        x+=step->x;
        mask=3;
        random=(random&mask)-2;
        x-=random;
        x=(u32)x<<16;
        x>>=16;
        random=RandomNumberGenerator();
        step=&steps[entity->stage];
        coordinate=(u16 *)&entity->posY.part16.integer;
        y=step->y+*coordinate;
        random=(random&mask)-2;
        y=(s16)(y-random);
        EventScene30_SpawnSparkBurstSmall(x,y);
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==10)entity->deleteRequested=1;
}
