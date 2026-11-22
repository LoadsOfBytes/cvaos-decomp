/* Player-relative directions, distance checks, angle and screen mirrors.
 * The neighboring aim helper has matching C in code_08068A24.c.
 * Names describe formulas; active source use is not asserted here. */
#include "structs/ewram.h"
#include "syscalls.h"

#define PLAYER_ENTITY (gEwramData->playerState.player)

/**
 * @brief 68A70 | Angle from entity to target.
 */
u16 EntityAngleTo(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    return ArcTan2(target->posX.part16.integer - entity->posX.part16.integer,
                   target->posY.part16.integer - entity->posY.part16.integer);
}

/**
 * @brief 68AA0 | Mirror an angle horizontally (0x8000 - angle).
 */
u16 AngleFlipX(u16 angle)
{
    return 0x8000 - angle;
}

/**
 * @brief 68AB4 | Mirror an angle vertically (-angle).
 */
u16 AngleFlipY(u16 angle)
{
    return -angle;
}

/**
 * @brief Return 240 pixels in fixed-point units minus the supplied
 * fixed-point X coordinate.
 */
s32 Screen_MirrorFixedX(s32 x)
{
    return (240 << 16) - x;
}

/**
 * @brief Return 160 pixels in fixed-point units minus the supplied
 * fixed-point Y coordinate.
 */
s32 Screen_MirrorFixedY(s32 y)
{
    return (160 << 16) - y;
}

/**
 * @brief 68AD4 | Whether the entity lies in the rectangle at (left,
 * top) of size (width, height) (16-bit wrap-around comparison).
 */
s32 Entity_IsInRect(struct EwramData_EntityData *entity, u16 left, u16 top, u16 width, u16 height)
{
    if ((u16)(entity->posX.part16.integer - left) <= width
        && (u16)(entity->posY.part16.integer - top) <= height)
        return 1;
    return 0;
}

/**
 * @brief Return 1 when the player is right of the entity, -1 when
 * left and 0 at equal fixed-point X.
 */
s32 Entity_GetPlayerDirectionX(struct EwramData_EntityData *entity)
{
    s32 playerX = PLAYER_ENTITY->posX.whole;

    if (entity->posX.whole < playerX)
        return 1;
    if (entity->posX.whole > playerX)
        return -1;
    return 0;
}

/**
 * @brief Return 1 when the player is below the entity, -1 when
 * above and 0 at equal fixed-point Y.
 */
s32 Entity_GetPlayerDirectionY(struct EwramData_EntityData *entity)
{
    s32 playerY = PLAYER_ENTITY->posY.whole;

    if (entity->posY.whole < playerY)
        return 1;
    if (entity->posY.whole > playerY)
        return -1;
    return 0;
}

/**
 * @brief Return 1 when integer-pixel Euclidean distance to the actual
 * player, shifted to fixed point and compared unsigned, is within
 * the supplied limit. This is distinct from the axis-range test.
 */
s32 Entity_IsPlayerWithinDistance(struct EwramData_EntityData *entity, u32 distance)
{
    struct EwramData_EntityData *player = PLAYER_ENTITY;
    s32 playerX = player->posX.whole;
    s32 playerY = player->posY.whole;
    s32 dx2;
    s32 dy2;

    dx2 = (entity->posX.whole - playerX) >> 16;
    dx2 *= dx2;
    dy2 = (entity->posY.whole - playerY) >> 16;
    dy2 *= dy2;
    if ((u32)(Sqrt(dx2 + dy2) << 16) <= distance)
        return 1;
    return 0;
}

/**
 * @brief 68BC0 | Shortest turning direction from angle from to to:
 * 0 when equal, -1 or 1 otherwise.
 */
s32 AngleTurnDirection(u16 from, u16 to)
{
    u16 diff = from - to;
    u16 back = to - from;

    if (from == to)
        return 0;
    if (diff > back)
        return 1;
    return -1;
}
