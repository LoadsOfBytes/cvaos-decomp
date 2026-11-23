#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern const s8 *const sDevilPartPoses[];
extern const s32 *const sDevilPartAngles[];

/* Interpolate a Devil joint toward its pose or its hanging angle. */
void Devil_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct DevilPart *parts = entity->ext.devil.parts;
    const s8 *pose = sDevilPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sDevilPartAngles[(s8)entity->ext.joint.pose];
    struct DevilPart *part = (struct DevilPart *)(index * sizeof(struct DevilPart) + (u32)parts);
    s32 x, y, dx, dy, a;
    if (part->targetSwingAngle == 0 && part->swingAngle == 0)
    {
        x = pose[index * 2] * 0x10000;
        y = pose[index * 2 + 1] * 0x10000;
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        x = angles[index];
        a = part->rotationAngle;
        x -= a;
        a += x / entity->timer;
        part->rotationAngle = a;
    }
    else
    {
        struct DevilPart *second = (struct DevilPart *)(index * sizeof(struct DevilPart) + (u32)parts);
        x = second->targetSwingAngle;
        a = second->swingAngle;
        x -= a;
        a += x / entity->timer;
        second->swingAngle = a;
        a += 0x4000;
        x = Sin(a);
        y = Sin(second->swingAngle);
        x = FixedMultiply(x, second->linkLength);
        y = FixedMultiply(y, second->linkLength);
        if (second->anchorXOrSentinel == (s32)0x80000000)
        {
            second->offsetX = parts[second->anchorYOrPartIndex].offsetX + x;
            second->offsetY = parts[second->anchorYOrPartIndex].offsetY + y;
        }
        else
        {
            second->offsetX = second->anchorXOrSentinel + x;
            second->offsetY = second->anchorYOrPartIndex + y;
        }
        x = angles[index];
        second = (struct DevilPart *)(index * sizeof(struct DevilPart));
        second = (struct DevilPart *)((u32)second + (u32)parts);
        a = second->rotationAngle;
        x -= a;
        a += x / entity->timer;
        second->rotationAngle = a;
    }
}
