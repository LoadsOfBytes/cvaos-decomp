/* Place nine timed affine fragments from the owner's saved origin. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "structs/ewram.h"
struct TimedFragment {u32 frame;s16 x,y;u8 delay;u8 pad[3];};
extern struct TimedFragment sEventScene30TimedFragments[9];
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30OrbitFragment_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn the nine delayed fragments of sEventScene30TimedFragments
 * (EventScene30OrbitFragment_Update) around the owner position.
 */
void EventScene30_SpawnOrbitFragments(struct EwramData_EntityData *owner)
{
    struct TimedFragment *item=sEventScene30TimedFragments;
    s32 index;
    for(index=0;index<9;index++,item++)
    {
        struct EwramData_EntityData *entity=EventScene30_CreateEffectSprite(1);
        if(entity)
        {
            entity->updateFunc=(u32 *)EventScene30OrbitFragment_Update;
            entity->posX.part16.integer=EVENT30_CENTER_X(owner)+item->x;
            entity->posY.part16.integer=EVENT30_CENTER_Y(owner)+item->y+32;
            entity->timer=item->delay;
            entity->animFrame=item->frame;
            EVENT30_FRAGMENT_PHASE(entity)=RandomNumberGenerator();
            EVENT30_FRAGMENT_DIR(entity)=index&1;
            EVENT30_FRAGMENT_INDEX(entity)=index;
            entity->hidden=1;
        }
    }
}
