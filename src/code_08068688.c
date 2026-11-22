/* Distance check and angle approach 0x08068688..0x08068787. Angle_TurnTowards only matches without
 * -f2003-patch (NO_2003_PATCH_SRCS in ../Makefile). */
#include "structs/ewram.h"

/**
 * @brief Return 1 when the player is within the supplied unsigned
 * fixed-point limit on both axes. This is an axis-aligned test,
 * not Euclidean distance; preserve coordinate load order.
 */
s32 Entity_IsPlayerWithinAxisRange(struct EwramData_EntityData *entity, u32 range)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 dist;

    // |entity - player| per axis; written as -player + entity for the load order
    dist = -player->posX.whole + entity->posX.whole;
    if (dist < 0)
        dist = player->posX.whole - entity->posX.whole;
    if ((u32)dist > range)
        return 0;
    dist = -player->posY.whole + entity->posY.whole;
    if (dist < 0)
        dist = player->posY.whole - entity->posY.whole;
    if ((u32)dist > range)
        return 0;
    return 1;
}

/* Direction to turn from angle param_0 to angle param_1: 0 when equal,
 * 1 when going up is shorter (the angles wrap at 0x10000), else -1. */
static inline s8 AngleTurnDirection(u16 from, u16 to)
{
    u16 down = from - to;
    u16 up = to - from;

    if (from == to)
        return 0;
    if (down > up)
        return 1;
    return -1;
}

/**
 * @brief 686CC | Turn the angle *angle by step towards target; 1 once
 * it reached target (it is then set exactly), else 0.
 */
u8 Angle_TurnTowards(u16 *angle, u16 step, u16 target)
{
    switch (AngleTurnDirection(*angle, target))
    {
        case 0:
            goto reached;
        case 1:
            *angle += step;
            if (AngleTurnDirection(*angle, target) == 1)
                goto turning;
            break;
        case -1:
            *angle -= step;
            if (AngleTurnDirection(*angle, target) == -1)
                goto turning;
            break;
        default:
            goto turning;
    }
    // overshot: snap to the target
    *angle = target;
reached:
    return 1;
turning:
    return 0;
}
