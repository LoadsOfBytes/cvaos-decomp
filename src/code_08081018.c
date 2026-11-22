/* Golem_AttachPart (joint set-up, variant of CreakingSkull_AttachJointSetAngle on struct
 * GolemPart). */
#include "syscalls.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 81018 | Hang part partIndex from part anchorIndex: store the parent,
 * the distance (length, or measured when 0), the angle to the parent + 0x10000
 * (unk_10) and the swing base unk_C from angleSpec (mirrored when facing
 * right); a angleSpec above 0xFFFF is an absolute angle, also
 * copied to unk_10.
 */
void Golem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct GolemPart *parts = entity->unk_500.golemParts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct GolemPart *part = (struct GolemPart *)(partIndex * sizeof(struct GolemPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct GolemPart *anglePart;

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
    anglePart = (struct GolemPart *)(partIndex * sizeof(struct GolemPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if (angleSpec > 0xFFFF)
    {
        if (entity->flipX)
            anglePart->targetSwingAngle = 0x28000 - angleSpec;
        else
            anglePart->targetSwingAngle = angleSpec;
        {
            struct GolemPart *absolutePart = (struct GolemPart *)(partIndex * sizeof(struct GolemPart) + (u32)parts);

            absolutePart->swingAngle = absolutePart->targetSwingAngle;
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
