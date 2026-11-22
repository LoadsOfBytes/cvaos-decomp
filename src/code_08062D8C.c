/* Place nine timed affine fragments from the owner's saved origin. */
#include "code_08009A0.h"
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
            entity->posX.part16.integer=owner->unk_504.part16.unk_504+item->x;
            entity->posY.part16.integer=owner->unk_504.part16.unk_506[0]+item->y+32;
            entity->timer=item->delay;
            entity->animFrame=item->frame;
            entity->unk_4FC.part16.unk_4FC=RandomNumberGenerator();
            entity->unk_4F8.part8.unk_4FA=index&1;
            entity->unk_4F8.part8.unk_4FB=index;
            entity->hidden=1;
        }
    }
}
