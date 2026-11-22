/* Let debris settle, flash near the end of its life and remove it. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
/**
 * @brief Falling body piece: from frame 40 down, slow horizontally, fall and blink;
 * delete when the timer ends.
 */
void GorgonPiece_Update(struct EwramData_EntityData *entity)
{
    if (entity->timer <= 40)
    {
        EntityDampVelocityX(entity, 8);
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer <= 40) entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    }
    if ((s8)--entity->timer <= 0) EntityDelete(entity);
}
