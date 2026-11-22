/* Entity_DrawWithVelocityCopies (motion trail draw). */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Draw the entity plus two copies 0.5 and 1.5 velocity steps ahead,
 * then restore its position.
 */
void Entity_DrawWithVelocityCopies(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + entity->velX.whole / 2;
    entity->posY.whole = oldY + entity->velY.whole / 2;
    for (i = 2; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
