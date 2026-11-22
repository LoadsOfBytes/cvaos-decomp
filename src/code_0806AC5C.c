/* Adjust a signed movement value for matching slope orientation. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern u8 Collision_GetTile(s32 x, s32 y);

static inline s32 AdjustForSlope(struct EwramData_EntityData *entity, s32 speed, s32 flags)
{
    s32 slope = flags & 0xC0;
    if (slope)
    {
        u32 kind = slope >> 6;
        s32 reversed = flags & 4;
        if (!entity->flipX)
        {
            if (reversed != 0)
                 return speed;
        }
        else if (reversed == 0)
            return speed;
        switch (kind)
        {
        case 1:  return speed / 2;
        case 2:  return speed * 2 / 3;
        default: asm(""); return speed; /* FAKEMATCH: changes jump2 order (cross-jump of the inlined copies) */
        }
    }
    return speed;
}

/**
 * @brief Scale the supplied signed speed for the matching tile slope
 * and facing: half speed for kind 1 or two thirds for kind 2.
 * If the current tile is zero, inspect one pixel below.
 */
s32 Entity_AdjustSpeedForSlope(struct EwramData_EntityData *entity, s32 speed)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 x = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
    s32 y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
    s32 flags = (u8)Collision_GetTile(x, y);
    s32 result;
    if (flags == 0)
        result = AdjustForSlope(entity, speed, (u8)Collision_GetTile(x, y + 1));
    else
         result = AdjustForSlope(entity, speed, flags);
    return result;
}
