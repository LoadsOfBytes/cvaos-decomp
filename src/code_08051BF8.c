/* Move an entity, settle it on the floor and tilt it along the slope
 * (Object14_MoveOnTerrain). The do/while (0) around the posY update doubles the weight
 * of those references so global-alloc orders off > d > dx (r5, r6, r7). */
#include "code_08001194.h"
#include "code/code_080211F0.h"
#include "syscalls.h"
#include "structs/ewram.h"
extern u8 Angle_TurnTowards(u16 *, u16, u16);

/**
 * @brief Move with velocity and gravity, snap onto the floor below and turn the
 * angle towards the slope ahead; return the floor push.
 */
s32 Object14_MoveOnTerrain(struct EwramData_EntityData *entity, s32 mode)
{
    s16 x, y, off, d, dx; s32 d2;

    off = 0;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velY.whole += entity->accelY.whole;
    Player_QueueEntity(entity, 7);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    if (entity->velY.whole == 0)
        off = 8;
    d = Collision_GetPushUpFiltered(x, y + off, mode);
    if (d != 0)
        do { entity->posY.whole += (d + off) << 16; } while (0);
    dx = 8;
    if (entity->velX.whole < 0)
        dx = -dx;
    d2 = Collision_GetPushUpFiltered(x + dx, y + off, mode);
    if (d2 != 0)
        { s32 angle = ArcTan2(dx, off + d2); Angle_TurnTowards((u16 *)&entity->unk_504, 0x80, angle); }
    return d;
}
