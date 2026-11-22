#include "code_08009A0.h"
#include "joint_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern const u8 *const sGolemPartPoses[];
extern const s32 *const sGolemPartAngles[];

/* Move golem body part `index` 1/timer of the way to its pose: without a
 * swing (unk_C and unk_10 zero) toward the kind's offset (mirrored with the
 * entity), otherwise turn the swing angle unk_10 toward unk_C and hang the
 * part at distance unk_1C from its anchor (part unk_18, or the point
 * unk_14/unk_18 unless unk_14 is 0x80000000). Parts 1, 3, 4, 5, 8 and 10
 * also turn toward their resting angle (cf. Stolas Stolas_InterpolateJoint). */
void Golem_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct GolemPart *parts = entity->unk_500.golemParts;
    const u8 *pose = sGolemPartPoses[(s8)JOINT_POSE(entity)];
    const s32 *angles = sGolemPartAngles[(s8)JOINT_POSE(entity)];
    s32 x, y, dx, dy;
    if (parts[index].targetSwingAngle == 0 && parts[index].swingAngle == 0)
    {
        struct GolemPart *part;
        if (entity->flipX) x = -((s8)pose[index * 2] * 0x10000);
        else x = (s8)pose[index * 2] * 0x10000;
        y = (s8)pose[index * 2 + 1] * 0x10000;
        part = (struct GolemPart *)(index * sizeof(struct GolemPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        if (index == 1 || index == 3 || index == 4 || index == 5 || index == 8 || index == 10)
        {
            s32 angle;
            if (index == 1) x = angles[0];
            else if (index == 3) x = angles[1];
            else if (index == 4) x = angles[2];
            else if (index == 5) x = angles[3];
            else if (index == 8) x = angles[4];
            else if (index == 10) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
    else
    {
        struct GolemPart *part = (struct GolemPart *)(index * sizeof(struct GolemPart) + (u32)parts);
        s32 swing;
        x = part->targetSwingAngle;
        swing = part->swingAngle;
        x -= swing;
        swing += x / entity->timer;
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
        if (index == 1 || index == 3 || index == 4 || index == 5 || index == 8 || index == 10)
        {
            s32 angle;
            if (index == 1) x = angles[0];
            else if (index == 3) x = angles[1];
            else if (index == 4) x = angles[2];
            else if (index == 5) x = angles[3];
            else if (index == 8) x = angles[4];
            else if (index == 10) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
}
