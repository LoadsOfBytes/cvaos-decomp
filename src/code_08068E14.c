/* Move along an angle and add a separate vertical displacement. */
#include "structs/ewram.h"
#include "code_08009A0.h"
/**
 * @brief Move along the supplied angle/speed and add vertical times
 * time to Y. Preserve the separate vertical term and shifted
 * sine products; this helper does not update velocity fields.
 */
void Entity_MoveAtAngleWithVerticalStep(struct EwramData_EntityData *entity, u16 angle, s32 speed, s16 vertical, u16 time)
{
    entity->posX.whole += (speed >> 8) * (Sin(angle + 0x4000) >> 8);
    entity->posY.whole += (speed >> 8) * (Sin(angle) >> 8) + vertical * time;
}
