/* Soul helpers 0x0802D510..0x0802D603. */
#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);

/**
 * @brief Return the selected target or first non-NULL candidate in cyclic order from a random slot; return NULL when empty, without promising uniform selection.
 */
struct EwramData_EntityData *Player_GetSelectedOrRandomTarget(void)
{
    struct EwramData_EntityData *target = gEwramData->playerState.selectedTarget;
    s32 i;
    u32 slot;

    if (target != NULL)
        return target;
    i = 0;
    slot = RandomNumberGenerator() & 3;
    for (; i <= 3; i++)
    {
        target = gEwramData->playerState.trackedEntities[slot];
        if (target != NULL)
            return target;
        slot = (slot + 1) & 3;
    }
    return NULL;
}

/**
 * @brief Return 1 (and set flag 0x53D bit 2) when the entity is outside the
 * screen by the given margin or its 0x53D bit 2 is clear; else 0.
 */
s32 Entity_IsOffscreenOrInactive(struct EwramData_EntityData *entity, s32 margin)
{
    if (EntityIsOutsideScreen(entity, margin) || !entity->roomActive)
    {
        entity->roomActive = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief 2D590 | Take the hit box of the current animation frame (spriteSlot /
 * animFrame): activate it with HitBox_SetAttack (flags bit 1 cleared), or set
 * flags bit 1 when the frame has none.
 */
void Entity_SetAttackBoxFromFrame(struct EwramData_EntityData *entity)
{
    u8 *spriteSlot = &entity->spriteSlot;
    struct SpriteBox *boxes = gEwramData->spriteSlots[*spriteSlot].sheet->frames[entity->animFrame].boxes;

    if (boxes != NULL)
    {
        entity->hit.flags &= ~2;
        HitBox_SetAttack(entity, boxes);
    }
    else
    {
        entity->hit.flags |= 2;
    }
}

/**
 * @brief 2D5EC | Random value in [-range / 2, range / 2) (range a
 * power of two).
 */
s32 RandomSigned(u32 range)
{
    return (RandomNumberGenerator() & (range - 1)) - (range >> 1);
}
