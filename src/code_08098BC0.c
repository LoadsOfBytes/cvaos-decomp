/* Creaking Skull body-part helpers 0x08098BC0..0x08098C83 (13 parts of
 * struct EntityJointPart, same code as the Stolas helpers in code_0809C304.c). */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

#define CREAKING_SKULL_PART_COUNT 13

extern void CreakingSkull_InterpolateJoint(struct EwramData_EntityData *entity, s32 index);

/**
 * @brief When timer is nonzero, interpolate joints 0..12 in numerical order.
 */
void CreakingSkull_InterpolateJoints(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < CREAKING_SKULL_PART_COUNT; i++)
            CreakingSkull_InterpolateJoint(entity, i);
    }
}

/**
 * @brief When timer is nonzero, interpolate thirteen joints in the supplied signed-byte index order.
 */
void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < CREAKING_SKULL_PART_COUNT; i++)
            CreakingSkull_InterpolateJoint(entity, order[i]);
    }
}

/**
 * @brief Clear the selected joint target and current hanging-angle fields.
 */
void CreakingSkull_ClearJointAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct EntityJointPart *parts = entity->unk_500.jointParts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief Save X/Y positions of all thirteen joints into their backup fields.
 */
void CreakingSkull_SaveJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->unk_500.jointParts;
    s32 i;

    for (i = CREAKING_SKULL_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief Restore X/Y positions of all thirteen joints from their backup fields.
 */
void CreakingSkull_RestoreJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->unk_500.jointParts;
    s32 i;

    for (i = CREAKING_SKULL_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/**
 * @brief Apply player contact damage using the entity attack value, zero effect fields and reaction field 1 at its current position.
 */
void CreakingSkull_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
