/* DiscArmorRing_Draw: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief B23C4 | Like Entity_DrawWithVelocityCopies, but 3 copies with the step unk_4FC / unk_500.
 */
void DiscArmorRing_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + (s32)entity->unk_4FC.whole / 2;
    entity->posY.whole = oldY + entity->unk_500.whole / 2;
    for (i = 3; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += (s32)entity->unk_4FC.whole;
        entity->posY.whole += entity->unk_500.whole;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
