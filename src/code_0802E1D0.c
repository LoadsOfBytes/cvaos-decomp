/* Update the attack box when animation advances to another sprite frame. */
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
static inline struct SpriteSheet *Graphics(u8 *index) { return gEwramData->spriteSlots[*index].sheet; }
/**
 * @brief Advance the animation; when the sprite frame changes, use its first
 * box as attack box and enable hits, or disable hits for a frame
 * without boxes. Used by four blue-soul update functions.
 */
void Anim_UpdateWithFrameAttackBox(struct EwramData_EntityData *entity)
{
    struct SpriteSheet *graphics = Graphics(&entity->spriteSlot);
    u8 frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame)
    {
        struct SpriteBox *box = graphics->frames[entity->animFrame].boxes;
        if (box != NULL)
        {
            entity->hit.flags &= ~2;
            HitBox_SetAttack(entity, box);
        }
        else entity->hit.flags |= 2;
    }
}
