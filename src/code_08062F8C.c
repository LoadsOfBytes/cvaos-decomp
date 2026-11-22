/* Create a falling affine particle linked to the parent's entity slot. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *EventScene30_CreateEffectSprite(s32);
extern void EventScene30FloatingFragment_Update(struct EwramData_EntityData *);
/**
 * @brief Spawn a falling then floating fragment
 * (EventScene30FloatingFragment_Update) at (x, y) with a random frame.
 */
void EventScene30_SpawnFloatingFragment(struct EwramData_EntityData *owner,s16 x,s16 y,s32 alternate)
{
    struct EwramData_EntityData *entity=EventScene30_CreateEffectSprite(1);
    if(entity)
    {
        u8 zero;
        entity->updateFunc=(u32 *)EventScene30FloatingFragment_Update;
        ENTITY_PARENT_INDEX(entity)=ENTITY_INDEX(owner);
        zero=0;
        EVENT30_EFFECT_SCALE(entity)=0x12000;
        entity->posX.part16.integer=x;entity->posY.part16.integer=y;
        entity->velX.whole=zero;entity->velY.whole=0x4000;
        entity->accelX.whole=zero;entity->accelY.whole=0x800;
        entity->spriteLayer=17;entity->timer=20;entity->group=1;
        if(!alternate)entity->animFrame=RandomNumberGenerator()%5+9;
        else entity->animFrame=RandomNumberGenerator()%7+2;
        /* Replace the packed render flags at entity + 0x3E. */
        {u16 flags=0x8000; *(u16 *)((u8 *)entity+0x3E)=flags;}
    }
}
