/* Nemesis_CreateAfterimages. */
#include "code_08009A0.h"
#include "nemesis_entity.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *entity, s32 delay, s32 count, s32 flags);

/**
 * @brief Create three afterimages with zero countdown, history indices 9, 12 and 6, and flag 2; store their slots.
 */
void Nemesis_CreateAfterimages(struct EwramData_EntityData *entity)
{
    NEMESIS_AFTERIMAGE_0(entity) = EntityGetSlot(Afterimage_Create(entity, 0, 9, 2));
    NEMESIS_AFTERIMAGE_1(entity) = EntityGetSlot(Afterimage_Create(entity, 0, 0xC, 2));
    NEMESIS_AFTERIMAGE_2(entity) = EntityGetSlot(Afterimage_Create(entity, 0, 6, 2));
}
