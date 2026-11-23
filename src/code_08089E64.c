/* Appear, follow a target with damping, then choose a new outward direction. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "syscalls.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Sin(s32);
extern s32 FixedMultiply(s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
static inline s32 Magnitude(s32 value)
{
    if (value < 0) return -value;
    return value;
}
static inline s32 TargetOffsetY(struct EwramData_EntityData *entity, s32 x)
{
    if (Magnitude(x) < 0x600000)
    {
        s32 distance = Target_GetY(entity) - entity->posY.whole;
        s32 offset = entity->ext.ghost.aimOffsetY - 0x100000;
        return distance + offset;
    }
    return Target_GetY(entity) - entity->posY.whole - 0x200000;
}
/**
 * @brief Float state (state 0): after the appear animation become hittable, then
 * drift toward the target with damped acceleration; when within 4 px bounce
 * away in a random direction (+-0x1000 around the angle from the target).
 */
void Ghost_UpdateFloat(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if (entity->animDone)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->hit.flags &= ~2;
            entity->hit.flags &= ~4;
            entity->ext.ghost.aimOffsetY = -(RandomNumberGenerator() & 0xFFFFF);
        }
        break;
    case 1:
    {
        s32 x, y;
        entity->flipX = Entity_IsTargetRight(entity);
        x = Target_GetX(entity) - entity->posX.whole;
        y = TargetOffsetY(entity, x);
        if (x > 0x40000) entity->accelX.whole = 0x400;
        else if (x < -0x40000) entity->accelX.whole = -0x400;
        else entity->accelX.whole = 0;
        if (y > 0x40000) entity->accelY.whole = 0x400;
        else if (y < -0x40000) entity->accelY.whole = -0x400;
        else entity->accelY.whole = 0;
        EntityDampVelocityX(entity, 32);
        EntityDampVelocityY(entity, 32);
        EntityAccelerateAndMove(entity, 0x10000);
        if (Magnitude(x) < 0x40000 && Magnitude(y) < 0x40000)
        {
            s32 pixelX, pixelY;
            s32 parameter;
            x = entity->posX.whole - Target_GetX(entity);
            y = entity->posY.whole - Target_GetY(entity) + 0x100000;
            if (x < 0) pixelX = (s16)(-((-x) >> 16));
            else pixelX = x >> 16;
            if (y < 0) pixelY = (s16)(-((-y) >> 16));
            else pixelY = y >> 16;
            parameter = ArcTan2(pixelX, pixelY);
            {
                s32 random = RandomNumberGenerator();
                s32 base = parameter - 0x1000;
                parameter = base + (random & 0x1FFF);
            }
            x = Sin(parameter + 0x4000);
            y = Sin(parameter);
            /* Reuse the consumed angle's temporary for the fixed-point speed. */
            parameter = 0x20000;
            entity->velX.whole = FixedMultiply(x, parameter);
            entity->velY.whole = FixedMultiply(y, parameter);
        }
        break;
    }
    }
}
