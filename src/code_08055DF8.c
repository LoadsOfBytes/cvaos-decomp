#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void WarpRoom_ResetGatePart(struct EwramData_EntityData *entity);

#define DIST(e) (WARP_GATE_DIST((e)))
#define SPEED(e) (WARP_GATE_SPEED((e)))
#define ACCEL(e) (WARP_GATE_ACCEL((e)))
#define POS(e) (WARP_GATE_POS((e)))
#define STEP(e) (WARP_GATE_STEP((e)))
#define HOMEX(e) (WARP_GATE_HOME_X((e)))
#define HOMEY(e) (WARP_GATE_HOME_Y((e)))

/**
 * @brief Animate a gate part through its opening and closing states, driven
 * by the warp room states 3 and 5.
 */
void WarpRoom_UpdateGatePart(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    struct EwramData_EntityData *parent = &ew->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 limit;

    switch (entity->state)
    {
    case 0:
        WarpRoom_ResetGatePart(entity);
        break;
    case 1:
        if (parent->state == 3)
            entity->state++;
        break;
    case 2:
        if (--entity->timer == 0xFF)
        {
            entity->affine = 1;
            HOMEX(entity) = entity->posX.part16.integer;
            HOMEY(entity) = entity->posY.part16.integer;
            entity->state++;
            entity->timer = 0;
        }
        break;
    case 3:
        if (entity->animFrame == 0)
        {
            if (entity->timer & 1)
            {
                u32 r = RandomNumberGenerator();
                s16 *p = (s16 *)&entity->posX.part16.integer;
                *p -= r % 3 - 1;
                r = RandomNumberGenerator();
                p = (s16 *)&entity->posY.part16.integer;
                *p -= r % 3 - 1;
            }
            else
            {
                entity->posX.part16.integer = HOMEX(entity);
                entity->posY.part16.integer = HOMEY(entity);
            }
        }
        if (entity->timer++ > 0x50)
        {
            entity->posX.part16.integer = HOMEX(entity);
            entity->posY.part16.integer = HOMEY(entity);
            entity->timer = 0;
            entity->state++;
        }
        break;
    case 4:
        entity->shrinkPartOffsets = 1;
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
        ObjAffine_Set(POS(entity), DIST(entity) + 0x1800, DIST(entity) + 0x1800, 0, 0, entity->affineIndex);
        if (DIST(entity) <= 0)
            entity->state++;
        break;
    case 5:
        entity->hidden = 1;
        break;
    case 6:
        if (parent->state == 5)
            entity->state++;
        break;
    case 7:
        if (--entity->timer == 0xFF)
        {
            entity->affine = 1;
            entity->hidden = 0;
            entity->timer = 0;
            entity->state++;
        }
        break;
    case 8:
        entity->shrinkPartOffsets = 1;
        DIST(entity) -= SPEED(entity);
        SPEED(entity) += ACCEL(entity);
        if (entity->animFrame > 3)
        {
            limit = 0;
            if (entity->animFrame > 5)
                limit = 5;
        }
        else
            limit = 0;
        if (entity->timer++ > limit)
            POS(entity) += STEP(entity);
        ObjAffine_Set(POS(entity), DIST(entity) + 0x1800, DIST(entity) + 0x1800, 0, 0, entity->affineIndex);
        if (POS(entity) == 0)
        {
            if (entity->animFrame != 0)
            {
                entity->state = 0;
            }
            else
            {
                entity->state = 0;
                parent->state++;
            }
            gEwramData->gameState.stateFlags &= ~0x40;
        }
        break;
    }
}
