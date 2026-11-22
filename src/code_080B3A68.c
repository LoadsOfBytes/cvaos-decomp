/* Column member settling. */
#include "structs/ewram.h"
#include "head_stack_entity.h"

/* old + (s16)(target - old): move a u16 coordinate onto target. */
static inline s32 Approach(s32 target, u16 old) { return (s16)(target - old) + old; }

/* posY of entity slot idx; the offset is computed before gEwramData is read. */
static inline s32 SlotY(u32 idx)
{
    return ((struct EwramData_EntityData *)(({ u32 o_ = idx * sizeof(struct EwramData_EntityData); o_; })
                                            + (u32)gEwramData + 0x4E4))->posY.part16.integer;
}

/**
 * @brief Fall onto a position 22 pixels above the highest occupied lower segment, or above the controller.
 */
void HeadStackSegment_Settle(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    u8 *slots = HEAD_STACK_SLOTS(owner);
    s32 i;
    s32 y;
    u16 old;

    for (i = HEAD_STACK_LEVEL(entity); i > 0; i--)
    {
        if (slots[i - 1] != 0)
        {
            y = SlotY(slots[i - 1]);
            goto found;
        }
    }
    y = owner->posY.part16.integer;
found:
    y -= 22;
    entity->posY.whole += entity->velY.whole;
    old = entity->posY.part16.integer;
    {
        s32 cur = entity->posY.part16.integer;

        /* in-place shifts (not (s16)y) keep the truncation in y's register */
        y <<= 16;
        y >>= 16;
        if (cur >= y)
        {
            entity->posY.part16.integer = Approach(y, old);
            entity->velY.whole = 0x1000;
        }
    }
    entity->velY.whole += 0x1000;
}
