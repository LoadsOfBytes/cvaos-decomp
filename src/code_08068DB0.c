/* Place an entity on a circle with a bounded angular rate. */
#include "structs/ewram.h"
#include "code_08009A0.h"
/**
 * @brief Set position around the supplied center and radius. Clamp
 * angle plus rate times time against the signed nonzero limit,
 * then multiply by time again to obtain the 16-bit direction.
 */
void Entity_SetCircularPositionAtTime(struct EwramData_EntityData *entity, s32 centerX, s32 centerY, u32 radius, s32 angle, s32 target, s32 rate, u16 time)
{
    u16 direction;
    s32 current = angle + rate * time;
    if ((target > 0 && current > target) || (target < 0 && current < target))
        current = target;
    direction = current * time;
    entity->posX.whole = centerX + (radius >> 8) * (Sin(direction + 0x4000) >> 8);
    entity->posY.whole = centerY + (radius >> 8) * (Sin(direction) >> 8);
}
