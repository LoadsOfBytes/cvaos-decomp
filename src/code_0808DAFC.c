#include "code_08009A0.h"
#include "joint_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern const u8 *const sGreatArmorPartPoses[];
extern const s32 *const sGreatArmorPartAngles[];

/* Move Great Armor body part `index` 1/timer of the way to its pose: without a
 * swing (unk_C and unk_10 zero) toward the kind's offset (mirrored with the
 * entity), otherwise turn the swing angle unk_10 toward unk_C and hang the
 * part at distance unk_24 from its anchor (part unk_18, or the point
 * unk_14/unk_18 unless unk_14 is 0x80000000). Parts 2, 5, 8, 11, 12 and 13
 * also turn toward their resting angle (clone of the golem Golem_UpdatePart). */
void GreatArmor_UpdatePart(struct EwramData_EntityData *entity, s32 index)
{
    struct GreatArmorPart *parts = entity->unk_500.greatArmorParts;
    const u8 *pose = sGreatArmorPartPoses[(s8)JOINT_POSE(entity)];
    const s32 *angles = sGreatArmorPartAngles[(s8)JOINT_POSE(entity)];
    s32 x, y, dx, dy;
    if (parts[index].targetSwingAngle == 0 && parts[index].swingAngle == 0)
    {
        struct GreatArmorPart *part;
        if (entity->flipX) x = -((s8)pose[index * 2] * 0x10000);
        else x = (s8)pose[index * 2] * 0x10000;
        y = (s8)pose[index * 2 + 1] * 0x10000;
        part = (struct GreatArmorPart *)(index * sizeof(struct GreatArmorPart) + (u32)parts);
        dx = part->offsetX;
        x -= dx;
        dy = part->offsetY;
        y -= dy;
        dx += x / entity->timer;
        part->offsetX = dx;
        dy += y / entity->timer;
        part->offsetY = dy;
        if (index == 2 || index == 5 || index == 8 || index == 11 || index == 12 || index == 13)
        {
            s32 angle;
            if (index == 2) x = angles[0];
            else if (index == 5) x = angles[1];
            else if (index == 8) x = angles[2];
            else if (index == 11) x = angles[3];
            else if (index == 12) x = angles[4];
            else if (index == 13) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
    else
    {
        struct GreatArmorPart *part = (struct GreatArmorPart *)(index * sizeof(struct GreatArmorPart) + (u32)parts);
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
        if (index == 2 || index == 5 || index == 8 || index == 11 || index == 12 || index == 13)
        {
            s32 angle;
            if (index == 2) x = angles[0];
            else if (index == 5) x = angles[1];
            else if (index == 8) x = angles[2];
            else if (index == 11) x = angles[3];
            else if (index == 12) x = angles[4];
            else if (index == 13) x = angles[5];
            if (entity->flipX) x = 0x20000 - x;
            angle = parts[index].rotationAngle;
            x -= angle;
            angle += x / entity->timer;
            parts[index].rotationAngle = angle;
        }
    }
}
