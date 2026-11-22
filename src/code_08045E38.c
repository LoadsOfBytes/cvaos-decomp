/* 5E38. */
#include "types.h"
#include "structs/ewram.h"

/* Put a live entity into state 3. */
/**
 * @brief Put a live entity into state 3.
 */
void Entity_EndIfActive(struct EwramData_EntityData *entity)
{
    if (entity->updateFunc != NULL)
        entity->state = 3;
}
