/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Stolas_AttachJoint = CreakingSkull_AttachJoint */
#include "syscalls.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Bind a joint to its parent with supplied or measured length; mirror the target angle by facing, treating absolute values above 0xFFFF in magnitude as absolute angles.
 */
void Stolas_AttachJoint(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct EntityJointPart *parts = entity->unk_500.jointParts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct EntityJointPart *part = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct EntityJointPart *anglePart;

    part->anchorXOrSentinel = 0x80000000;
    part->anchorYOrPartIndex = anchorIndex;
    if (length != 0)
    {
        part->linkLength = length;
    }
    else
    {
        dx = parts[anchorIndex].offsetX - part->offsetX;
        dy = parts[anchorIndex].offsetY - part->offsetY;
        dx = FixedMultiply(dx, dx);
        dy = FixedMultiply(dy, dy);
        part->linkLength = (u32)(Sqrt(dx + dy) << 16) >> 8;
    }
    dx = parts[partIndex].offsetX - parts[anchorIndex].offsetX;
    dy = parts[partIndex].offsetY - parts[anchorIndex].offsetY;
    angle = ArcTan2(dx < 0 ? -(-dx >> 16) : dx >> 16, dy < 0 ? -(-dy >> 16) : dy >> 16);
    anglePart = (struct EntityJointPart *)(partIndex * sizeof(struct EntityJointPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
    {
        if (entity->flipX)
        {
            if (angleSpec > 0x18000)
                anglePart->targetSwingAngle = 0x38000 - angleSpec;
            else
                anglePart->targetSwingAngle = 0x28000 - angleSpec;
        }
        else
        {
            anglePart->targetSwingAngle = angleSpec;
        }
    }
    else if (entity->flipX)
    {
        anglePart->targetSwingAngle = baseAngle - angleSpec;
    }
    else
    {
        anglePart->targetSwingAngle = baseAngle + angleSpec;
    }
}
