#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern const u8 *const sStolasPartPoses[];
extern const s32 *const sStolasPartAngles[];

/* Interpolate a Stolas joint toward a pose or its hanging angle; parts 2
 * and 5 also turn toward the pose angles (cf. Creaking Skull CreakingSkull_InterpolateJoint). */
/**
 * @brief Interpolate one of six joints toward pose coordinates or a hanging angle using the entity timer; also interpolate pose rotations of joints 2 and 5.
 */
void Stolas_InterpolateJoint(struct EwramData_EntityData *entity, s32 index)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    const u8 *pose = sStolasPartPoses[(s8)entity->ext.joint.pose];
    const s32 *angles = sStolasPartAngles[(s8)entity->ext.joint.pose];
    s32 x, y, dx, dy, a;
    struct EntityJointPart *part;
    s32 *p5;
    if (parts[index].targetSwingAngle == 0 && parts[index].swingAngle == 0)
    {
        if (entity->flipX) x = -((s8)pose[index * 2] * 0x10000);
        else x = (s8)pose[index * 2] * 0x10000;
        y = (s8)pose[index * 2 + 1] * 0x10000;
        part = (struct EntityJointPart *)(index * sizeof(struct EntityJointPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        if (index == 2)
        {
            x = angles[0];
            if (entity->flipX) x = 0x20000 - x;
            a = parts[2].rotationAngle;
            x -= a;
            a += x / entity->timer;
            parts[2].rotationAngle = a;
        }
        else if (index == 5)
        {
            x = angles[1];
            if (entity->flipX) x = 0x20000 - x;
            p5 = &parts[5].rotationAngle;
            a = *p5;
            x -= a;
            a += x / entity->timer;
            *p5 = a;
        }
    }
    else
    {
        struct EntityJointPart *second = (struct EntityJointPart *)(index * sizeof(struct EntityJointPart) + (u32)parts);
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
            second->offsetX = (parts[second->anchorYOrPartIndex].offsetX & 0xFFFF0000) + x;
            second->offsetY = (parts[second->anchorYOrPartIndex].offsetY & 0xFFFF0000) + y;
        }
        else
        {
            second->offsetX = second->anchorXOrSentinel + x;
            second->offsetY = second->anchorYOrPartIndex + y;
        }
        if (index == 2)
        {
            x = angles[0];
            if (entity->flipX) x = 0x20000 - x;
            a = parts[2].rotationAngle;
            x -= a;
            a += x / entity->timer;
            parts[2].rotationAngle = a;
        }
        else if (index == 5)
        {
            x = angles[1];
            if (entity->flipX) x = 0x20000 - x;
            p5 = &parts[5].rotationAngle;
            a = *p5;
            x -= a;
            a += x / entity->timer;
            *p5 = a;
        }
    }
}
