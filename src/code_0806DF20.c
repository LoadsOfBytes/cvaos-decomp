/* Entity_AnimateWithBoxes. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);

/**
 * @brief 6DF20 | Animate; refresh the boxes (Entity_UpdateBoxes) when the frame
 * id animFrame changed.
 */
void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity)
{
    u8 frame = entity->animFrame;

    Anim_Update(entity);
    if (frame != entity->animFrame)
        Entity_UpdateBoxes(entity);
}
