/* Fragments slow down, start falling, blink and expire. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern void EntityMove(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
extern s32 Entity_GetVisibilityPattern(s32);
static inline s32 Magnitude(s32 value)
{
    if(value<0) value=-value;
    return value;
}
/**
 * @brief Loose feather: slow down, then drift down for 40 frames while blinking
 * out.
 */
void HarpyDeathFeather_Update(struct EwramData_EntityData *entity)
{
    s32 state=entity->state;
    switch(state)
    {
        case 0:
            EntityDampVelocityX(entity,12);
            EntityDampVelocityY(entity,12);
            EntityMove(entity);
            if(Magnitude(entity->velX.whole)<=0xFFF && Magnitude(entity->velY.whole)<=0xFFF)
            {
                entity->state++;
                entity->animFrame=8;
                entity->velX.whole=state;
                entity->velY.whole=0x4000;
            }
            break;
        case 1:
            EntityMove(entity);
            if(++entity->timer>39)
                EntityDelete(entity);
            entity->hidden=Entity_GetVisibilityPattern(40-entity->timer);
            break;
    }
}
