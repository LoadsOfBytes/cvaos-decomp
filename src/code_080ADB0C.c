#include "code_08009A0.h"
#include "code_08001194.h"
#include "structs/ewram.h"
/* Probe up to four vertical offsets and adjust entity Y on the first hit.
 * Keep the original u16 X result and its later separate signed conversion.
 * The correction local and short register lifetimes preserve agbcc's
 * arithmetic order; a match does not prove original source spelling. */
void Worm_SettleOnGround(struct EwramData_EntityData *entity)
{
    s32 offset = 0xFFF8;
    if (!gEwramData->gameState.bestiaryPreview)
    {
        s32 i = 0;
        s16 *posY = (s16 *)&entity->posY.part16.integer;
        for (; i <= 3; i++)
        {
            u16 x = GetEntityRoomXPositionInteger(entity);
            u16 y = GetEntityRoomYPositionInteger(entity);
            s32 gap;
            s32 probeOffset;
            s32 signedX = (s16)x;
            { s32 signedY = (s16)y;
            offset = (s16)offset;
            probeOffset = offset;
            probeOffset += 10;
            signedY += probeOffset;
            gap = Collision_GetPushUp(signedX, signedY); }
            if (gap != 0)
            {
                register s32 correction asm("r0") = offset + 2; /* FAKEMATCH: pinned register */
                correction = gap + correction;
                *posY = correction + *posY;
                break;
            }
            offset = (u16)(offset + 16);
        }
    }
}
