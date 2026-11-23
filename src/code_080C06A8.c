/* Slime terrain support points and angle interpolation. */
#include "code_08009A0.h"
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
    u16 angle = entity->ext.slime.angle;
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
    if (entity->ext.slime.speed <= 0)
    {
        dx = -(u32)((u32)dx << 16) >> 16;
        dy = -(u32)((u32)dy << 16) >> 16;
    }
    centerX = (s16)roomX;
    tangentX = (s16)dx;
    rightX = centerX + tangentX;
    perpendicularX = (s16)normalX;
    entity->ext.slime.frontSupportX = (u32)((perpendicularX + rightX) << 16) >> 16;
    centerY = (s16)roomY;
    tangentY = (s16)dy;
    rightY = centerY + tangentY;
    perpendicularY = (s16)normalY;
    entity->ext.slime.frontSupportY = (u32)((perpendicularY + rightY) << 16) >> 16;
    entity->ext.slime.backSupportX = perpendicularX + (centerX - tangentX);
    entity->ext.slime.backSupportY = perpendicularY + (centerY - tangentY);
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
        height = (s16)Slime_GetFloorCorrection(entity->ext.slime.frontSupportX, entity->ext.slime.frontSupportY + 6);
        if (height != 0) entity->ext.slime.frontSupportY = (u32)((height + (u16)entity->ext.slime.frontSupportY) << 16) >> 16;
        height = (s16)Slime_GetFloorCorrection(entity->ext.slime.backSupportX, entity->ext.slime.backSupportY + 6);
        if (height != 0) entity->ext.slime.backSupportY = (u32)((height + (u16)entity->ext.slime.backSupportY) << 16) >> 16;
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
        height = (s16)Collision_GetPushDown(entity->ext.slime.frontSupportX, entity->ext.slime.frontSupportY - 6);
        if (height != 0) entity->ext.slime.frontSupportY = (u32)((height + (u16)entity->ext.slime.frontSupportY) << 16) >> 16;
        height = (s16)Collision_GetPushDown(entity->ext.slime.backSupportX, entity->ext.slime.backSupportY - 6);
        if (height != 0) entity->ext.slime.backSupportY = (u32)((height + (u16)entity->ext.slime.backSupportY) << 16) >> 16;
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
        height = (s16)Collision_GetPushLeft(entity->ext.slime.frontSupportX + 6, entity->ext.slime.frontSupportY);
        if (height != 0) entity->ext.slime.frontSupportX = (u32)((height + (u16)entity->ext.slime.frontSupportX) << 16) >> 16;
        height = (s16)Collision_GetPushLeft(entity->ext.slime.backSupportX + 6, entity->ext.slime.backSupportY);
        if (height != 0) entity->ext.slime.backSupportX = (u32)((height + (u16)entity->ext.slime.backSupportX) << 16) >> 16;
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
        height = (s16)Collision_GetPushRight(entity->ext.slime.frontSupportX - 6, entity->ext.slime.frontSupportY);
        if (height != 0) entity->ext.slime.frontSupportX = (u32)((height + (u16)entity->ext.slime.frontSupportX) << 16) >> 16;
        height = (s16)Collision_GetPushRight(entity->ext.slime.backSupportX - 6, entity->ext.slime.backSupportY);
        if (height != 0) entity->ext.slime.backSupportX = (u32)((height + (u16)entity->ext.slime.backSupportX) << 16) >> 16;
        break;
    }
    if (entity->ext.slime.speed > 0)
    {
        edgeX = (s16)((u16)entity->ext.slime.frontSupportX - (u16)entity->ext.slime.backSupportX);
        {
            firstY = (u16)entity->ext.slime.frontSupportY;
            secondY = (u16)entity->ext.slime.backSupportY;
        }
    }
    else
    {
        edgeX = (s16)((u16)entity->ext.slime.backSupportX - (u16)entity->ext.slime.frontSupportX);
        {
            firstY = (u16)entity->ext.slime.backSupportY;
            secondY = (u16)entity->ext.slime.frontSupportY;
        }
    }
    firstY -= secondY;
    angle = ArcTan2(edgeX, (s16)firstY);
    Angle_TurnTowards((u16 *)&entity->ext.slime.angle, step, angle);
    return lostGround;
}
