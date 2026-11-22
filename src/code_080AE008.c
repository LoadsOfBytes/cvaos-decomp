/* Worm_GetSlopeAngle: matched (s16 coordinates: stored zero-extended, sign-extended
   at use; loop 1 argument y1 + (i + 10) kept unassociated). */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "syscalls.h"
#include "structs/ewram.h"
/**
 * @brief Return the angle of the floor under the entity from probes 8 px to each
 * side.
 */
s32 Worm_GetSlopeAngle(struct EwramData_EntityData *e)
{
    s16 x = GetEntityRoomXPositionInteger(e);
    s16 y1 = GetEntityRoomYPositionInteger(e);
    s16 x2 = x + 8;
    s16 y2 = y1;
    s32 i;
    s16 x1 = x - 8;
    s16 h;
    for (i = 0; i < 32; i += 8)
    {
        h = Collision_GetPushUp(x1, y1 + ({ s32 t_ = i + 10; t_; }));
        if (h != 0)
        {
            y1 = y1 + h + i;
            break;
        }
    }
    for (i = 0; i < 32; i += 8)
    {
        h = Collision_GetPushUp(x2, y2 + i + 10);
        if (h != 0)
        {
            y2 = y2 + h + i;
            break;
        }
    }
    return (u16)ArcTan2(x1 - x2, y1 - y2) + 0x8000;
}
