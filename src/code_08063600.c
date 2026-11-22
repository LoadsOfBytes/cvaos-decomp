/* Five fixed effect positions, each activated after its own delay. */
#include "code_08009A0.h"
#include "structs/ewram.h"
struct EffectStep { u16 x,y; u8 delay; u8 padding[3]; };
struct EffectSequence { struct EffectStep steps[5]; };
extern struct EffectSequence sEventScene30EmitterCSequence;
extern void EventScene30_SpawnSparkBurst(s16,s16);
/**
 * @brief Emitter callback 2: step through sEventScene30EmitterCSequence spawning spark bursts.
 */
void EventScene30EmitterC_Update(struct EwramData_EntityData *entity)
{
    struct EffectSequence sequence=sEventScene30EmitterCSequence;
    s32 stage=entity->state;
    if((u8)stage==0)
    {
        entity->timer=sequence.steps[entity->stage].delay;
        entity->state=stage+1;
    }
    else if((u8)stage==1)
    {
        if(--entity->timer==255) entity->state=stage+1;
    }
    else
    {
        struct EffectStep *step=&sequence.steps[entity->stage];
        EventScene30_SpawnSparkBurst(step->x+entity->posX.part16.integer,step->y+(u16)entity->posY.part16.integer);
        entity->state=0;
        entity->stage++;
    }
    if(entity->stage==5)entity->deleteRequested=1;
}
