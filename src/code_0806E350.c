/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Entity_PlaceRelativeToCopy = Entity_PlaceRelativeTo */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Same as Entity_PlaceRelativeTo.
 */
void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, u16 offsetY)
{
    s16 dx = offsetX;
    s16 dy = offsetY;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = (s32)dy + anchor->posY.part16.integer;
}
