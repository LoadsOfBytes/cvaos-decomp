/* Small accessors (2A54, 2A6C). */
#include "types.h"
#include "structs/ewram.h"
#include "structs/entity_boxes.h"

/* Set byte index of the table at EWRAM 0x1307C. */
void Hit_SetTimerOverride(s32 index, s32 value)
{
    gEwramData->hitTimerOverride[index] = value;
}

/* Box record of the entity's slot (boxSlot). */
/**
 * @brief Return the EntityBoxes record indexed by the entity hit boxSlot, retaining unchecked indexing.
 */
struct EntityBoxes *Hit_GetEntityBoxes(struct EwramData_EntityData *entity)
{
    return &gEntityBoxes[entity->hit.boxSlot];
}
