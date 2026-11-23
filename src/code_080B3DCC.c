/* Stack keeper. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

/**
 * @brief Set the controller visibility flag, discard dying segment slots and select the highest live segment; delete an empty stack.
 */
void HeadStack_Update(struct EwramData_EntityData *entity)
{
    s32 found = 0;
    u8 *slot;
    s32 i;

    if (!Entity_IsInRect(entity, -32, -32, 0x130, (u16)(entity->spawnParam0 * 32 + 0xE0)))
    {
        entity->ext.headStack.onScreen = 0;
        return;
    }
    entity->ext.headStack.onScreen = 1;
    slot = (entity->spawnParam0 + entity->ext.headStack.slots) - 1;
    for (i = entity->spawnParam0; i > 0; slot--, i--)
    {
        if (*slot != 0)
        {
            struct EwramData_EntityData *e = &gEwramData->entityData[*slot];

            if (e->state == 2)
                *slot = 0;
            else if (!found)
            {
                Player_QueueEntity(e, 0);
                found = 1;
            }
        }
    }
    if (!found)
        EntityDelete(entity);
}
