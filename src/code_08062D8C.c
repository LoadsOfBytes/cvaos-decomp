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
            entity->posX.part16.integer=owner->ext.event30.centerX+item->x;
            entity->posY.part16.integer=owner->ext.event30.centerY+item->y+32;
            entity->timer=item->delay;
            entity->animFrame=item->frame;
            entity->ext.event30Fragment.phase=RandomNumberGenerator();
            entity->ext.event30Fragment.dir=index&1;
            entity->ext.event30Fragment.index=index;
            entity->hidden=1;
        }
    }
}
