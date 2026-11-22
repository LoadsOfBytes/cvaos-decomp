/* Slime terrain support points and angle interpolation. */
#include "code_08009A0.h"
#include "slime_entity.h"
#include "code_08001194.h"
#include "syscalls.h"
extern s32 Slime_GetFloorCorrection(s32, s32);
extern u8 Angle_TurnTowards(u16 *, u16, u16);
#define SLIME_FIXED_INTEGER(v) ((u32)((v) < 0 ? -((-(v)) >> 16) << 16 : (v)) >> 16)

/**
 * @brief Probe center and two support points on the current surface, adjust
 * position and interpolate the tangent angle. Return nonzero if the
 * center probe loses support; retain the signed/halfword coordinate arithmetic.
 */
s32 Slime_UpdateSurfaceSupport(struct EwramData_EntityData *entity, u16 step)
{
    u16 angle = SLIME_ANGLE(entity);
    u16 roomX;
    u16 roomY;
    s32 lostGround = 0;
    u16 dx;
    u16 dy;
    u16 normalX;
    u16 normalY;
    s32 height;
    s32 centerX;
    s32 centerY;
    s32 tangentX;
    s32 tangentY;
    s32 perpendicularX;
    s32 perpendicularY;
    s32 rightX;
    s32 rightY;
    s32 edgeX;
    s32 firstY;
    s32 secondY;
    roomX = (u16)GetEntityRoomXPositionInteger(entity);
    roomY = (u16)GetEntityRoomYPositionInteger(entity);
    dx = SLIME_FIXED_INTEGER(6 * Sin((s16)angle + 0x4000));
    dy = SLIME_FIXED_INTEGER(6 * Sin((s16)angle));
    angle = (((u32)angle << 16) + 0x40000000) >> 16;
    normalX = SLIME_FIXED_INTEGER(6 * Sin((s16)angle + 0x4000));
    normalY = SLIME_FIXED_INTEGER(6 * Sin((s16)angle));
    if (SLIME_SPEED(entity) <= 0)
    {
        dx = -(u32)((u32)dx << 16) >> 16;
        dy = -(u32)((u32)dy << 16) >> 16;
    }
    centerX = (s16)roomX;
    tangentX = (s16)dx;
    rightX = centerX + tangentX;
    perpendicularX = (s16)normalX;
    SLIME_FRONT_SUPPORT_X(entity) = (u32)((perpendicularX + rightX) << 16) >> 16;
    centerY = (s16)roomY;
    tangentY = (s16)dy;
    rightY = centerY + tangentY;
    perpendicularY = (s16)normalY;
    SLIME_FRONT_SUPPORT_Y(entity) = (u32)((perpendicularY + rightY) << 16) >> 16;
    SLIME_BACK_SUPPORT_X(entity) = perpendicularX + (centerX - tangentX);
    SLIME_BACK_SUPPORT_Y(entity) = perpendicularY + (centerY - tangentY);
    switch (entity->stage)
    {
    case 0:
        {
            s32 centerHeight = (s16)Slime_GetFloorCorrection(centerX, centerY + 0xB);
            if (centerHeight != 0)
            {
                s32 value = (u16)entity->posY.part16.integer;
                value += 6;
                entity->posY.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Slime_GetFloorCorrection(SLIME_FRONT_SUPPORT_X(entity), SLIME_FRONT_SUPPORT_Y(entity) + 6);
        if (height != 0) SLIME_FRONT_SUPPORT_Y(entity) = (u32)((height + (u16)SLIME_FRONT_SUPPORT_Y(entity)) << 16) >> 16;
        height = (s16)Slime_GetFloorCorrection(SLIME_BACK_SUPPORT_X(entity), SLIME_BACK_SUPPORT_Y(entity) + 6);
        if (height != 0) SLIME_BACK_SUPPORT_Y(entity) = (u32)((height + (u16)SLIME_BACK_SUPPORT_Y(entity)) << 16) >> 16;
        break;
    case 2:
        {
            s32 centerHeight = (s16)Collision_GetPushDown(centerX, centerY - 0xB);
            if (centerHeight != 0)
            {
                s32 value = (u16)entity->posY.part16.integer;
                value -= 6;
                entity->posY.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Collision_GetPushDown(SLIME_FRONT_SUPPORT_X(entity), SLIME_FRONT_SUPPORT_Y(entity) - 6);
        if (height != 0) SLIME_FRONT_SUPPORT_Y(entity) = (u32)((height + (u16)SLIME_FRONT_SUPPORT_Y(entity)) << 16) >> 16;
        height = (s16)Collision_GetPushDown(SLIME_BACK_SUPPORT_X(entity), SLIME_BACK_SUPPORT_Y(entity) - 6);
        if (height != 0) SLIME_BACK_SUPPORT_Y(entity) = (u32)((height + (u16)SLIME_BACK_SUPPORT_Y(entity)) << 16) >> 16;
        break;
    case 1:
        {
            s32 centerHeight = (s16)Collision_GetPushLeft(centerX + 0xB, centerY);
            if (centerHeight != 0)
            {
                s32 value = entity->posX.part16.integer;
                value += 6;
                entity->posX.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Collision_GetPushLeft(SLIME_FRONT_SUPPORT_X(entity) + 6, SLIME_FRONT_SUPPORT_Y(entity));
        if (height != 0) SLIME_FRONT_SUPPORT_X(entity) = (u32)((height + (u16)SLIME_FRONT_SUPPORT_X(entity)) << 16) >> 16;
        height = (s16)Collision_GetPushLeft(SLIME_BACK_SUPPORT_X(entity) + 6, SLIME_BACK_SUPPORT_Y(entity));
        if (height != 0) SLIME_BACK_SUPPORT_X(entity) = (u32)((height + (u16)SLIME_BACK_SUPPORT_X(entity)) << 16) >> 16;
        break;
    case 3:
        {
            s32 centerHeight = (s16)Collision_GetPushRight(centerX - 0xB, centerY);
            if (centerHeight != 0)
            {
                s32 value = entity->posX.part16.integer;
                value -= 6;
                entity->posX.part16.integer = centerHeight + value;
            }
            else lostGround = 1;
        }
        GetEntityRoomXPositionInteger(entity);
        GetEntityRoomYPositionInteger(entity);
        height = (s16)Collision_GetPushRight(SLIME_FRONT_SUPPORT_X(entity) - 6, SLIME_FRONT_SUPPORT_Y(entity));
        if (height != 0) SLIME_FRONT_SUPPORT_X(entity) = (u32)((height + (u16)SLIME_FRONT_SUPPORT_X(entity)) << 16) >> 16;
        height = (s16)Collision_GetPushRight(SLIME_BACK_SUPPORT_X(entity) - 6, SLIME_BACK_SUPPORT_Y(entity));
        if (height != 0) SLIME_BACK_SUPPORT_X(entity) = (u32)((height + (u16)SLIME_BACK_SUPPORT_X(entity)) << 16) >> 16;
        break;
    }
    if (SLIME_SPEED(entity) > 0)
    {
        edgeX = (s16)((u16)SLIME_FRONT_SUPPORT_X(entity) - (u16)SLIME_BACK_SUPPORT_X(entity));
        {
            firstY = (u16)SLIME_FRONT_SUPPORT_Y(entity);
            secondY = (u16)SLIME_BACK_SUPPORT_Y(entity);
        }
    }
    else
    {
        edgeX = (s16)((u16)SLIME_BACK_SUPPORT_X(entity) - (u16)SLIME_FRONT_SUPPORT_X(entity));
        {
            firstY = (u16)SLIME_BACK_SUPPORT_Y(entity);
            secondY = (u16)SLIME_FRONT_SUPPORT_Y(entity);
        }
    }
    firstY -= secondY;
    angle = ArcTan2(edgeX, (s16)firstY);
    Angle_TurnTowards((u16 *)&SLIME_ANGLE(entity), step, angle);
    return lostGround;
}
