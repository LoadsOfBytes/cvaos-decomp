#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

#define DIST(e) ((e)->unk_4F8.whole)
#define SPEED(e) ((e)->unk_4FC.part16.unk_4FE[0])
#define ACCEL(e) ((e)->unk_4FC.part16.unk_4FC)
#define POS(e) ((e)->unk_504.part16.unk_504)
#define STEP(e) ((e)->unk_504.part16.unk_506[0])

/**
 * @brief Reset a gate part to its closed animation; on arrival (stateFlags bit
 * 0x40) fast-forward it to the open state 6.
 */
void WarpRoom_ResetGatePart(struct EwramData_EntityData *entity)
{
    s32 limit;

    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x48;
    entity->state = 1;
    if (entity->animFrame == 0)
        entity->timer = entity->animFrame * 5;
    else
        entity->timer = entity->animFrame * 5 + 15;
    DIST(entity) = 0xFFFF;
    SPEED(entity) = 10;
    ACCEL(entity) = 0x3C;
    POS(entity) = 0;
    STEP(entity) = 0x800;
    ObjAffine_Set(0, 0x10000, 0x10000, 0, 0, entity->affineIndex);
    entity->shrinkPartOffsets = 0;
    if (!(gEwramData->gameState.stateFlags & 0x40))
        return;
    entity->timer = 0;
    do
    {
        DIST(entity) -= SPEED(entity);
        SPEED(entity) += ACCEL(entity);
        if (entity->animFrame > 3)
        {
            limit = 20;
            if (entity->animFrame > 5)
                limit = 10;
        }
        else
            limit = 30;
        if (entity->timer++ > limit)
            POS(entity) += STEP(entity);
    } while (DIST(entity) > 0);
    entity->state = 6;
    STEP(entity) *= -1;
    SPEED(entity) *= -1;
    ACCEL(entity) *= -1;
    if (entity->animFrame == 0)
        entity->timer = (9 - entity->animFrame) * 5 + 15;
    else
        entity->timer = (9 - entity->animFrame) * 5;
    ObjAffine_Set(POS(entity), DIST(entity) + 0x1800, DIST(entity) + 0x1800, 0, 0, entity->affineIndex);
    entity->affine = 1;
}
