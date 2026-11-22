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
    u16 angle = entity->unk_504.part16.unk_504;
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
    if (entity->unk_4F8.whole <= 0)
    {
        dx = -(u32)((u32)dx << 16) >> 16;
        dy = -(u32)((u32)dy << 16) >> 16;
    }
    centerX = (s16)roomX;
    tangentX = (s16)dx;
    rightX = centerX + tangentX;
    perpendicularX = (s16)normalX;
    entity->unk_508.part16.unk_508 = (u32)((perpendicularX + rightX) << 16) >> 16;
    centerY = (s16)roomY;
    tangentY = (s16)dy;
    rightY = centerY + tangentY;
    perpendicularY = (s16)normalY;
    entity->unk_508.part16.unk_50A = (u32)((perpendicularY + rightY) << 16) >> 16;
    entity->unk_50C.part16.unk_50C = perpendicularX + (centerX - tangentX);
    entity->unk_50C.part16.unk_50E = perpendicularY + (centerY - tangentY);
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
        height = (s16)Slime_GetFloorCorrection(entity->unk_508.part16.unk_508, entity->unk_508.part16.unk_50A + 6);
        if (height != 0) entity->unk_508.part16.unk_50A = (u32)((height + (u16)entity->unk_508.part16.unk_50A) << 16) >> 16;
        height = (s16)Slime_GetFloorCorrection(entity->unk_50C.part16.unk_50C, entity->unk_50C.part16.unk_50E + 6);
        if (height != 0) entity->unk_50C.part16.unk_50E = (u32)((height + (u16)entity->unk_50C.part16.unk_50E) << 16) >> 16;
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
        height = (s16)Collision_GetPushDown(entity->unk_508.part16.unk_508, entity->unk_508.part16.unk_50A - 6);
        if (height != 0) entity->unk_508.part16.unk_50A = (u32)((height + (u16)entity->unk_508.part16.unk_50A) << 16) >> 16;
        height = (s16)Collision_GetPushDown(entity->unk_50C.part16.unk_50C, entity->unk_50C.part16.unk_50E - 6);
        if (height != 0) entity->unk_50C.part16.unk_50E = (u32)((height + (u16)entity->unk_50C.part16.unk_50E) << 16) >> 16;
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
        height = (s16)Collision_GetPushLeft(entity->unk_508.part16.unk_508 + 6, entity->unk_508.part16.unk_50A);
        if (height != 0) entity->unk_508.part16.unk_508 = (u32)((height + (u16)entity->unk_508.part16.unk_508) << 16) >> 16;
        height = (s16)Collision_GetPushLeft(entity->unk_50C.part16.unk_50C + 6, entity->unk_50C.part16.unk_50E);
        if (height != 0) entity->unk_50C.part16.unk_50C = (u32)((height + (u16)entity->unk_50C.part16.unk_50C) << 16) >> 16;
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
        height = (s16)Collision_GetPushRight(entity->unk_508.part16.unk_508 - 6, entity->unk_508.part16.unk_50A);
        if (height != 0) entity->unk_508.part16.unk_508 = (u32)((height + (u16)entity->unk_508.part16.unk_508) << 16) >> 16;
        height = (s16)Collision_GetPushRight(entity->unk_50C.part16.unk_50C - 6, entity->unk_50C.part16.unk_50E);
        if (height != 0) entity->unk_50C.part16.unk_50C = (u32)((height + (u16)entity->unk_50C.part16.unk_50C) << 16) >> 16;
        break;
    }
    if (entity->unk_4F8.whole > 0)
    {
        edgeX = (s16)((u16)entity->unk_508.part16.unk_508 - (u16)entity->unk_50C.part16.unk_50C);
        {
            firstY = (u16)entity->unk_508.part16.unk_50A;
            secondY = (u16)entity->unk_50C.part16.unk_50E;
        }
    }
    else
    {
        edgeX = (s16)((u16)entity->unk_50C.part16.unk_50C - (u16)entity->unk_508.part16.unk_508);
        {
            firstY = (u16)entity->unk_50C.part16.unk_50E;
            secondY = (u16)entity->unk_508.part16.unk_50A;
        }
    }
    firstY -= secondY;
    angle = ArcTan2(edgeX, (s16)firstY);
    Angle_TurnTowards((u16 *)&entity->unk_504.part16.unk_504, step, angle);
    return lostGround;
}
