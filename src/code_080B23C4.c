/* DiscArmorRing_Draw: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code/code_08039340.h"
#include "disc_armor_entity.h"
#include "structs/ewram.h"

/**
 * @brief B23C4 | Like Entity_DrawWithVelocityCopies, but 3 copies with the step unk_4FC / unk_500.
 */
void DiscArmorRing_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    entity->posX.whole = oldX + (s32)entity->ext.discArmorRing.stepX / 2;
    entity->posY.whole = oldY + entity->ext.discArmorRing.stepY / 2;
    for (i = 3; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += (s32)entity->ext.discArmorRing.stepX;
        entity->posY.whole += entity->ext.discArmorRing.stepY;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
