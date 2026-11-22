/* BaloreEmitter_Draw: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief B6BF4 | Like Entity_DrawWithVelocityCopies, but 8 copies with the step unk_50C / unk_510.
 */
void BaloreEmitter_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + entity->unk_50C.whole / 2;
    entity->posY.whole = oldY + entity->unk_510.whole / 2;
    for (i = 8; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->unk_50C.whole;
        entity->posY.whole += entity->unk_510.whole;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
