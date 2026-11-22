/* Nightmare_Draw. */
#include "code/code_08039340.h"
#include "nightmare_entity.h"
#include "structs/ewram.h"

/**
 * @brief Draw at the cached room coordinates relative to background 1,
 * then restore both full position values.
 */
void Nightmare_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    entity->posX.part16.integer = NIGHTMARE_ROOM_X(entity) - bg->xPos.part16.integer;
    entity->posY.part16.integer = NIGHTMARE_ROOM_Y(entity) - bg->yPos.part16.integer;
    Sprite_DrawEntity(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
