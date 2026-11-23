/* BaloreEmitter_Draw: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code/code_08039340.h"
#include "balore_entity.h"
#include "structs/ewram.h"

/**
 * @brief B6BF4 | Like Entity_DrawWithVelocityCopies, but 8 copies with the step BALORE_EMITTER_STEP_X / _Y.
 */
void BaloreEmitter_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + BALORE_EMITTER_STEP_X(entity) / 2;
    entity->posY.whole = oldY + BALORE_EMITTER_STEP_Y(entity) / 2;
    for (i = 8; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += BALORE_EMITTER_STEP_X(entity);
        entity->posY.whole += BALORE_EMITTER_STEP_Y(entity);
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
