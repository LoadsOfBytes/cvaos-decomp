/* DevilPart_Attach (Devil joint set-up, like CreakingSkull_AttachJoint on struct DevilPart
 * without mirroring). */
#include "syscalls.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Bind a DevilPart to its parent with supplied or measured length; set measured angle and absolute-or-relative target angle without facing mirroring.
 */
void DevilPart_Attach(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length)
{
    struct DevilPart *parts = entity->unk_500.devilParts;
    // &parts[partIndex], written as integer arithmetic (offset + base) as in
    // the original
    struct DevilPart *part = (struct DevilPart *)(partIndex * sizeof(struct DevilPart) + (u32)parts);
    s32 dx;
    s32 dy;
    s32 baseAngle;
    s32 angle;
    struct DevilPart *anglePart;

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
    anglePart = (struct DevilPart *)(partIndex * sizeof(struct DevilPart) + (u32)parts);
    anglePart->swingAngle = baseAngle = (u16)angle + 0x10000;
    if ((angleSpec < 0 ? -angleSpec : angleSpec) > 0xFFFF)
        anglePart->targetSwingAngle = angleSpec;
    else
        anglePart->targetSwingAngle = baseAngle + angleSpec;
}
