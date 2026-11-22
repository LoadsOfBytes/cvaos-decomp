/* Legion steering uses a partial view of the state allocation reached through
 * entity->unk_4F8. Room-space position words are at 0x0C/0x10; retain this
 * padded view and the original pointer casts. */
#include "code_08009A0.h"
extern void EntityAccelerateX(struct EwramData_EntityData *, s32);
extern void EntityAccelerateY(struct EwramData_EntityData *, s32);

struct LegionRoomPositionView {
    u8 pad_0[0xC];
    s32 roomX;
    s32 roomY;
};

/**
 * @brief Accelerate the path position x (parent word 0x0C) toward target.
 */
void Legion_SteerX(struct EwramData_EntityData *entity, s32 target)
{
    struct LegionRoomPositionView *parent = (void *)entity->unk_4F8.unk_4F8_entity;
    if (parent->roomX < target) entity->accelX.whole = 0x400;
    else entity->accelX.whole = -0x400;
    EntityAccelerateX(entity, 0x4000);
    parent->roomX += entity->velX.whole;
}

/**
 * @brief Accelerate the path position y (parent word 0x10) toward target.
 */
void Legion_SteerY(struct EwramData_EntityData *entity, s32 target)
{
    struct LegionRoomPositionView *parent = (void *)entity->unk_4F8.unk_4F8_entity;
    if (parent->roomY < target) entity->accelY.whole = 0x400;
    else entity->accelY.whole = -0x400;
    EntityAccelerateY(entity, 0x4000);
    parent->roomY += entity->velY.whole;
}
