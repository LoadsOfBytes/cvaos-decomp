/* Object0F_Draw (same code as SaveSelect_Open). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 50F08 | Animate, then Sprite_Draw.
 */
void Object0F_Draw(struct EwramData_EntityData *entity)
{
    Anim_Update(entity);
    Sprite_Draw(entity);
}
