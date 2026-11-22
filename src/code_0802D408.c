/* Step toward a fixed-point target angle and wrap at 0x10000.
 * The step is not clamped to the remaining angle; equal angles retain the
 * original subtract branch. Target pixel conversion truncates toward zero. */
#include "structs/ewram.h"
#include "syscalls.h"
#define INT_TOWARD_ZERO(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)
/**
 * @brief Add or subtract the supplied turn toward the target angle and wrap to 16 bits; preserve equal-angle/tie branches and possible overshoot without snapping.
 */
u32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *entity, s32 x, s32 y, s32 angle, s32 turn)
{
    s32 dx = (s16)(INT_TOWARD_ZERO(x) - (s16)entity->posX.part16.integer);
    s32 dy = (s16)(INT_TOWARD_ZERO(y) - entity->posY.part16.integer);
    s32 delta = (u16)ArcTan2(dx, dy) - angle;
    if (delta > 0)
    {
        if (delta <= 0x7FFF) goto add;
        goto subtract;
    }
    else if (delta >= -0x8000) goto subtract;
add:
    angle += turn;
    goto finish;
subtract:
    angle -= turn;
finish:
    return angle & 0xFFFF;
}
