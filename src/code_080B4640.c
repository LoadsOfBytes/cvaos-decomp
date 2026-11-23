/* Steered flight towards a point next to entity unk_508. */
#include "code/code_08039340.h"
#include "balore_entity.h"
#include "structs/ewram.h"

/* Flight parameters, e.g. sBaloreHandSteeringParams1 (BaloreHand_FadeIn). */
struct BaloreFlightSteeringParameters {
    /* 0x00 */ u16 offsetX;     // point relative to unk_508/unk_50A, mirrored when not flipX
    /* 0x02 */ u16 offsetY;
    /* 0x04 */ u16 targetAngle; // for unk_500, negated when not flipX
    /* 0x06 */ u16 turnStep;
    /* 0x08 */ s32 targetRadius;
    /* 0x0C */ s32 radiusStep;
    /* 0x10 */ s32 targetSpeed;
    /* 0x14 */ s32 stepSpeed;
};

extern u8 Angle_TurnTowards(u16 *, u16, u16);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *, s32, s32, s32);
extern void EntityMove(struct EwramData_EntityData *);

/* the field is typed as a pointer in the shared union */
#define SPEED(e) (*(s32 *)&(e)->unk_514)

/**
 * @brief B4640 | Step the speed unk_514 and the value unk_4F4 towards the
 * targets of @p p (clamped, never overshooting), turn the angle unk_500 by
 * turnStep towards targetAngle and aim the velocity at the point
 * (unk_508 + offsetX, unk_50A + offsetY); offsetX and targetAngle are
 * mirrored when the entity is not flipX.  Moves the entity (EntityMove) and
 * returns 0 while on the way; returns 1 without moving once the point is
 * within reach.
 */
s32 Entity_SteerToPoint(struct EwramData_EntityData *entity, const struct BaloreFlightSteeringParameters *p)
{
    s32 target, rate;
    s16 dx, dy, angle;
    u16 x, y;

    target = p->targetSpeed;
    rate = p->stepSpeed;
    if (SPEED(entity) != target)
    {
        if (SPEED(entity) > target)
        {
            if ((SPEED(entity) -= rate) < target)
                SPEED(entity) = target;
        }
        else if (SPEED(entity) < target)
        {
            if ((SPEED(entity) += rate) > target)
                SPEED(entity) = target;
        }
    }
    target = p->targetRadius;
    rate = p->radiusStep;
    if (entity->ext.baloreHand.armLength != target)
    {
        if (entity->ext.baloreHand.armLength > target)
        {
            if ((entity->ext.baloreHand.armLength -= rate) < target)
                entity->ext.baloreHand.armLength = target;
        }
        else if (entity->ext.baloreHand.armLength < target)
        {
            if ((entity->ext.baloreHand.armLength += rate) > target)
                entity->ext.baloreHand.armLength = target;
        }
    }
    dx = p->offsetX;
    dy = p->offsetY;
    angle = p->targetAngle;
    if (!entity->flipX)
    {
        dx = -dx;
        angle = -angle;
    }
    /* separate s16 offsets and u16 results decide the register allocation */
    x = dx + entity->ext.baloreHand.homeX;
    y = dy + entity->ext.baloreHand.homeY;
    Angle_TurnTowards((u16 *)&entity->ext.baloreHand.armAngle, p->turnStep, angle);
    if (Entity_AimVelocityAtPoint(entity, x << 16, y << 16, SPEED(entity)) != 0)
        return 1;
    EntityMove(entity);
    return 0;
}
