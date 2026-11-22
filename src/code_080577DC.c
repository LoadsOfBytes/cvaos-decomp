/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object2A_Draw = Object34Part_Draw */
#include "code_08009A0.h"
#include "layer_object.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/**
 * @brief 577DC | Same as Object34Part_Draw.
 */
void Object2A_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = BG1_OBJECT_DRAW_X(entity);
    entity->posY.whole = BG1_OBJECT_DRAW_Y(entity);
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
