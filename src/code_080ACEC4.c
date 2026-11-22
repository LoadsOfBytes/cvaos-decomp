/* Big Golem body-part helpers 0x080ACEC4..0x080AD00B. */
#include "structs/ewram.h"

#define BIG_GOLEM_PART_COUNT 14

extern void BigGolem_UpdatePart(struct EwramData_EntityData *entity, s32 index);
extern void BigGolem_AttachPartAbsolute(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);
extern void BigGolem_AttachPart(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration);

/**
 * @brief ACEC4 | Move the entity by the mean x of parts partIndex0 and partIndex1
 * and shift all parts back by the same amount.
 */
void BigGolem_RecenterOnParts(struct EwramData_EntityData *entity, s32 partIndex0, s32 partIndex1)
{
    struct BigGolemPart *parts = entity->unk_500.bigGolemParts;
    s32 shift = (parts[partIndex0].offsetX + parts[partIndex1].offsetX) / 2;
    s32 i;

    entity->posX.whole += shift;
    for (i = BIG_GOLEM_PART_COUNT - 1; i >= 0; parts++, i--)
        parts->offsetX -= shift;
}

/**
 * @brief ACF00 | Set the y offset of part partIndex.
 */
void BigGolem_SetPartYOffset(struct EwramData_EntityData *entity, s32 partIndex, s32 offsetY)
{
    entity->unk_500.bigGolemParts[partIndex].offsetY = offsetY;
}

/**
 * @brief ACF10 | Unless timer is 0, run BigGolem_UpdatePart on parts 0..13.
 */
void BigGolem_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < BIG_GOLEM_PART_COUNT; i++)
            BigGolem_UpdatePart(entity, i);
    }
}

/**
 * @brief ACF30 | Unless timer is 0, run BigGolem_UpdatePart on the 14 parts
 * listed in order.
 */
void BigGolem_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < BIG_GOLEM_PART_COUNT; i++)
            BigGolem_UpdatePart(entity, order[i]);
    }
}

/**
 * @brief Clear the swing base targetSwingAngle and angle swingAngle of part index.
 */
void BigGolem_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct BigGolemPart *parts = entity->unk_500.bigGolemParts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief ACF6C | BigGolem_AttachPartAbsolute with half of duration, then set easingState = 1 and
 * interpolationDurationLimit = duration of part partIndex.
 */
void BigGolem_AttachPartAbsoluteEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration)
{
    struct BigGolemPart *parts;

    BigGolem_AttachPartAbsolute(entity, partIndex, anchorIndex, angleSpec, length, duration / 2);
    parts = entity->unk_500.bigGolemParts;
    parts[partIndex].easingState = 1;
    parts[partIndex].interpolationDurationLimit = duration;
}

/**
 * @brief ACFA0 | Same as BigGolem_AttachPartAbsoluteEased with BigGolem_AttachPart.
 */
void BigGolem_AttachPartEased(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length, s32 duration)
{
    struct BigGolemPart *parts;

    BigGolem_AttachPart(entity, partIndex, anchorIndex, angleSpec, length, duration / 2);
    parts = entity->unk_500.bigGolemParts;
    parts[partIndex].easingState = 1;
    parts[partIndex].interpolationDurationLimit = duration;
}

/**
 * @brief ACFD4 | Save the offsets of all parts (offsetX/offsetY -> savedOffsetX/savedOffsetY).
 */
void BigGolem_SaveParts(struct EwramData_EntityData *entity)
{
    struct BigGolemPart *parts = entity->unk_500.bigGolemParts;
    s32 i;

    for (i = BIG_GOLEM_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief ACFF0 | Restore the offsets saved by BigGolem_SaveParts.
 */
void BigGolem_RestoreParts(struct EwramData_EntityData *entity)
{
    struct BigGolemPart *parts = entity->unk_500.bigGolemParts;
    s32 i;

    for (i = BIG_GOLEM_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}
