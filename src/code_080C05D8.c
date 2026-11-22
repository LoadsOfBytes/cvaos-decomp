/* Slime terrain query and turn-speed setup. */
#include "code_08001194.h"
extern const u16 sSlimeSurfaceTurnTargets[];
extern s32 __divsi3(s32, s32);

/**
 * @brief Query upward terrain correction and filter the resulting collision
 * flags. Return the signed correction for accepted terrain, otherwise zero;
 * the exact material interpretation of the tested flag combinations is unresolved.
 */
s32 Slime_GetFloorCorrection(s32 x, s32 y)
{
    u16 height = Collision_GetPushUp(x, y);
    s32 flags = gUnk_03002CB0.collisionProbeFlags;
    s32 valid = 0;
    s32 type;
    s32 result;
    if (flags & 2) goto solid;
    if (flags & 0xC0)
    {
solid:
        if (flags & 8)
        {
            if (flags & 1) type = 2;
            else type = 1;
        }
        else type = 0;
        if (type != 1) valid = 1;
    }
    if (!valid) result = 0;
    else result = (s16)height;
    return result;
}

