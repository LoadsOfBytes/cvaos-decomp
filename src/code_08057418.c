/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Object29_Draw = Object34Part_Draw */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/**
 * @brief 57418 | Same as Object34Part_Draw.
 */
void Object29_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.whole = entity->ext.layerObject.drawX;
    entity->posY.whole = entity->ext.layerObject.drawY;
    Sprite_Draw(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
