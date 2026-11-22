#include "code_08009A0.h"
#include "joint_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern const s16 *const sBigGolemPartPoses[];
extern const s32 *const sBigGolemPartAngles[];

/* Big Golem joint step: like the golem Golem_UpdatePart, but every part has its
 * own countdown interpolationTimer as divisor and a swing state easingState (1: wait for half
 * of interpolationDuration, 2: slow down until interpolationDuration reaches interpolationDurationLimit). Parts 5 and 12 also
 * turn toward their resting angle. */
void BigGolem_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct BigGolemPart *parts = entity->unk_500.bigGolemParts;
    const s16 *pose = sBigGolemPartPoses[(s8)JOINT_POSE(entity)];
    const s32 *angles = sBigGolemPartAngles[(s8)JOINT_POSE(entity)];
    s32 x, y, dx, dy, timer;
    struct BigGolemPart *p = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
    timer = p->interpolationTimer;
    if (timer == 0) timer = 1;
    p->interpolationTimer--;
    if (p->targetSwingAngle == 0 && p->swingAngle == 0)
    {
        struct BigGolemPart *part;
        if (entity->flipX) x = -(pose[index * 2] * 0x10000);
        else x = pose[index * 2] * 0x10000;
        y = pose[index * 2 + 1] * 0x10000;
        part = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / timer;
        part->offsetX = dx;
        dy += y / timer;
        part->offsetY = dy;
        if (index == 5 || index == 12)
        {
            s32 angle;
            if (index == 5) x = angles[0];
            else if (index == 12) x = angles[1];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / timer;
            parts[index].rotationAngle = angle;
        }
    }
    else
    {
        struct BigGolemPart *part = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
        s32 swing;
        if (part->easingState == 0)
        {
            x = part->targetSwingAngle;
            swing = part->swingAngle;
            x -= swing;
            swing += x / timer;
            part->swingAngle = swing;
            swing += 0x4000;
            x = Sin(swing);
            y = Sin(part->swingAngle);
            x = FixedMultiply(x, part->linkLength);
            y = FixedMultiply(y, part->linkLength);
            if (part->anchorXOrSentinel == (s32)0x80000000)
            {
                part->offsetX = parts[part->anchorYOrPartIndex].offsetX + x;
                part->offsetY = parts[part->anchorYOrPartIndex].offsetY + y;
            }
            else
            {
                part->offsetX = part->anchorXOrSentinel + x;
                part->offsetY = part->anchorYOrPartIndex + y;
            }
            if (index == 5 || index == 12)
            {
                s32 angle;
                if (index == 5) x = angles[0];
                else if (index == 12) x = angles[1];
                if (entity->flipX) x = 0x20000 - x;
                angle = parts[index].rotationAngle;
                x -= angle;
                angle += x / timer;
                parts[index].rotationAngle = angle;
            }
        }
        else
        {
            if (part->easingState == 1)
            {
                if (part->interpolationTimer <= part->interpolationDuration / 2)
                    part->easingState = 2;
            }
            else if (part->easingState == 2)
            {
                part->interpolationTimer++;
                part->interpolationDuration++;
                timer = part->interpolationTimer;
                if (part->interpolationDuration >= part->interpolationDurationLimit)
                    part->easingState = 3;
            }
            part = (struct BigGolemPart *)(index * sizeof(struct BigGolemPart) + (u32)parts);
            x = part->targetSwingAngle;
            swing = part->swingAngle;
            x -= swing;
            swing += x / timer;
            part->swingAngle = swing;
            swing += 0x4000;
            x = Sin(swing);
            y = Sin(part->swingAngle);
            x = FixedMultiply(x, part->linkLength);
            y = FixedMultiply(y, part->linkLength);
            if (part->anchorXOrSentinel == (s32)0x80000000)
            {
                part->offsetX = parts[part->anchorYOrPartIndex].offsetX + x;
                part->offsetY = parts[part->anchorYOrPartIndex].offsetY + y;
            }
            else
            {
                part->offsetX = part->anchorXOrSentinel + x;
                part->offsetY = part->anchorYOrPartIndex + y;
            }
            if (index == 5 || index == 12)
            {
                s32 angle;
                if (index == 5) x = angles[0];
                else if (index == 12) x = angles[1];
                if (entity->flipX) x = 0x20000 - x;
                angle = parts[index].rotationAngle;
                x -= angle;
                angle += x / timer;
                parts[index].rotationAngle = angle;
            }
        }
    }
}
