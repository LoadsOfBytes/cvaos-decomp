/* Lunge towards the target. */
#include "code/code_08039340.h"
#include "nemesis_entity.h"
#include "code_080D73B8.h"
#include "code_08009A0.h"
#include "code_08001194.h"
#include "structs/ewram.h"

extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *);
extern void Nemesis_UpdateRotatedBoxes(struct EwramData_EntityData *);

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Spin in place, aim toward the target, lunge for the computed timer and return to positioning; update rotation and boxes throughout.
 */
void Nemesis_UpdateSpinLunge(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 targetX, targetY, dist;

    targetX = Target_GetX(entity) + bg->xPos.whole;
    targetY = Target_GetY(entity) + bg->yPos.whole - 0x200000;
    /* computed but unused in the original */
    dist = ABS((s32)(targetX - GetEntityRoomXPositionWhole(entity)));
    dist = ABS((s32)(targetY - GetEntityRoomYPositionWhole(entity)));
    Nemesis_UpdateRotatedBoxes(entity);
    switch (entity->subState)
    {
    case 0:
        entity->accelY.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->velX.whole = 0;
        NEMESIS_TURN(entity) = 0x1000;
        entity->timer = 0;
        entity->subState++;
    case 1:
        if (!(entity->timer % 16))
            PlaySong(0x85);
        if (entity->timer++ > 0x40)
        {
            NEMESIS_TURN(entity) = 0;
            NEMESIS_ANGLE(entity) = (u32)Entity_GetTargetAngleAbove16(entity) + 0xC000;
            entity->velX.whole = FixedMultiply(Sin(NEMESIS_ANGLE(entity) + 0x8000), 0x20000);
            entity->velY.whole = FixedMultiply(Sin(NEMESIS_ANGLE(entity) + 0x4000), 0x20000);
            entity->timer = ABS((Target_GetX(entity) - entity->posX.whole) / entity->velX.whole);
            entity->subState++;
            PlaySong(0x85);
        }
        break;
    case 2:
        if (--entity->timer == 0)
        {
            entity->subState = 3;
            entity->timer = 0;
        }
        break;
    case 3:
    case 4:
        if (entity->state != 1)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
    NEMESIS_ANGLE(entity) += NEMESIS_TURN(entity);
    ObjAffine_Set(NEMESIS_ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
