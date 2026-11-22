#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "structs/ewram.h"
static inline s8 FindFreeSoulSlot(void)
{
    s8 i;
    for (i = 0; i <= 7; i++)
        if (gEwramData->playerState.activeBlueSoulEntities[i] == NULL) return i;
    return -1;
}
/**
 * @brief Create a blue soul entity with the given update in slots 2..16 and
 * register it in the first free active blue soul slot; NULL when no slot or
 * entity is free.
 */
struct EwramData_EntityData *BlueSoul_CreateEntity(struct EwramData_EntityData *owner, u32 *update)
{
    s8 slot = FindFreeSoulSlot();
    struct EwramData_EntityData *entity;
    if (slot < 0)
    {
fail:
        return NULL;
    }
    entity = EntityCreateInRange(2, 16, update);
    if (entity == NULL) goto fail;
    BLUE_SOUL_OWNER(entity) = owner;
    BLUE_SOUL_SLOT(entity) = slot;
    gEwramData->playerState.activeBlueSoulEntities[slot] = entity;
    entity->roomActive = 1;
    return entity;
}
