/* Particle movement toward its owner, retaining the original termination checks.
 * The owner register and cached X coordinate preserve agbcc's register use.
 * Matching does not establish the original source spelling. */
#include "code_08009A0.h"
#include "structs/ewram.h"

/**
 * @brief Step toward the owner using the configured signed pixel step.
 * Remove on the existing X or particle-relative Y test, or timer
 * underflow. Keep the pre-move Y comparison and cached coordinates.
 */
void BelmontPowerParticle_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner =
        &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s8 step = entity->unk_4FC.part8.unk_4FC;
    u16 initialYMinus32 = entity->posY.part16.integer - 32;

    if ((s16)owner->posX.part16.integer > (s16)entity->posX.part16.integer)
        entity->posX.part16.integer += step;
    else
        entity->posX.part16.integer -= step;
    if ((s16)owner->posY.part16.integer - 32 > (s16)entity->posY.part16.integer)
        entity->posY.part16.integer += step;
    else
        entity->posY.part16.integer -= step;
    {
        s32 ownX = (s16)entity->posX.part16.integer;
        s32 ownerX = (s16)owner->posX.part16.integer;

        /* The Y window uses this particle's position before movement,
         * minus 32, rather than the owner's Y coordinate. */
        if ((ownX < ownerX + 4 && ownX > ownerX - 4) ||
            ((s16)entity->posY.part16.integer < (s16)initialYMinus32 + 4 &&
             (s16)entity->posY.part16.integer > (s16)initialYMinus32 - 4))
            do { EntityDelete(entity); } while (0);
        else if (--entity->timer == 255)
            EntityDelete(entity);
    }
}
