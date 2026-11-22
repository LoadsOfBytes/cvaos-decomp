/* Entity motion with tile collision; profile has count, top, bottom,
 * half-width, then count vertical sample offsets. */
#include "structs/ewram.h"
#include "structs/main.h"
#include "code_08001194.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *);

/**
 * @brief Move using profile count/top/bottom/half-width and vertical
 * wall samples, optional floor filtering and step size. Return
 * wall/ceiling/floor/slope bits and store the landing tile flags.
 */
s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *entity, s16 *profile, u16 stepSize, s32 flags)
{
    s32 count = profile[0];
    s16 *sample = profile + 4;
    s32 step = 0;
    s32 result = 0;
    u16 x = GetEntityRoomXPositionInteger(entity);
    u16 y = GetEntityRoomYPositionInteger(entity);
    s32 limit = 0x80000;
    s32 limitY;
    s32 zero;
    s32 i;
    u8 *tileFlags;

    entity->velX.whole += entity->accelX.whole;
    if (entity->accelX.whole >= 0)
    {
        if (entity->velX.whole > limit)
            entity->velX.whole = limit;
    }
    else if (entity->velX.whole < -0x80000)
        entity->velX.whole = -0x80000;
    entity->posX.whole += entity->velX.whole;
    if (entity->velX.whole < 0)
    {
        i = 0;
        if (i < count)
        {
            s32 sampleX = (s16)x;
            s32 sampleY = (s16)y;
            do
            {
                s32 delta = Collision_GetPushRight(sampleX - profile[3], sampleY + *sample);
                if (delta != 0)
                {
                    entity->posX.whole += delta << 16;
                    result |= 1;
                    break;
                }
                sample++;
                i++;
            } while (i < count);
        }
    }
    else if (entity->velX.whole > 0)
    {
        i = 0;
        if (i < count)
        {
            s32 sampleX = (s16)x;
            s32 sampleY = (s16)y;
            do
            {
                s32 delta = Collision_GetPushLeft(profile[3] + sampleX, sampleY + *sample);
                if (delta != 0)
                {
                    entity->posX.whole += delta << 16;
                    result |= 2;
                    break;
                }
                sample++;
                i++;
            } while (i < count);
        }
    }
    limitY = 0x80000;
    entity->velY.whole += entity->accelY.whole;
    if (entity->accelY.whole >= 0)
    {
        if (entity->velY.whole > limitY)
            entity->velY.whole = limitY;
    }
    else if (entity->velY.whole < -0x80000)
        entity->velY.whole = -0x80000;
    entity->posY.whole += entity->velY.whole;
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    tileFlags = &entity->floorFlags;
    zero = 0;
    *tileFlags = zero;
    if (entity->velY.whole < 0)
    {
        s32 delta = Collision_GetPushDown((s16)x, (s16)y + profile[1]);
        if (delta != 0)
        {
            s32 correction = delta << 16;
            entity->posY.whole += correction;
            result |= 4;
            entity->velY.whole = zero;
        }
    }
    else
    {
        u32 delta;
        if (entity->velY.part16.integer == 0)
            step = stepSize;
        delta = (u16)Collision_GetPushUp((s16)x, (s16)y + profile[2] + (s16)step);
        {
            s32 filterMask = 2;
            if (flags & filterMask)
            {
                s32 tile = gUnk_03002CB0.collisionProbeFlags;
                s32 allowed = 0;
                if ((tile & filterMask) || (tile & 0xC0))
                {
                    s32 kind;
                    if (tile & 8)
                        kind = (tile & 1) ? 2 : 1;
                    else
                        kind = 0;
                    if (kind != 1)
                        allowed = 1;
                }
                if (!allowed)
                    goto finish;
            }
        }
        if ((s16)delta == 0)
            goto finish;
        entity->posY.whole += ((s16)delta + (s16)step) << 16;
        result |= 8;
        entity->velY.whole = 0;
        entity->floorFlags = gUnk_03002CB0.collisionProbeFlags;
        if (gUnk_03002CB0.collisionProbeFlags & 0xC0)
        {
            result |= 0x10;
            if ((entity->velX.whole > 0 && (gUnk_03002CB0.collisionProbeFlags & 4))
                || (entity->velX.whole < 0 && !(gUnk_03002CB0.collisionProbeFlags & 4)))
                result |= 0x40;
            else
                result |= 0x20;
        }
        if ((flags & 1) && Collision_GetPushUp((s16)x, (s16)y + profile[2]) != 0
            && (gUnk_03002CB0.collisionProbeFlags & 0xC0))
            result |= 0x10;
    }
finish:
    return result;
}

