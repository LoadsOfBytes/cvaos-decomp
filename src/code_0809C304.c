/* Stolas body-part helpers 0x0809C304..0x0809C41B. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

#define STOLAS_PART_COUNT 6

extern void Stolas_InterpolateJoint(struct EwramData_EntityData *entity, s32 index);

/**
 * @brief When timer is nonzero, interpolate joints 0..5 in numerical order.
 */
void Stolas_InterpolateJoints(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < STOLAS_PART_COUNT; i++)
            Stolas_InterpolateJoint(entity, i);
    }
}

/**
 * @brief When timer is nonzero, interpolate six joints in the supplied signed-byte index order.
 */
void Stolas_InterpolateJointsInOrder(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < STOLAS_PART_COUNT; i++)
            Stolas_InterpolateJoint(entity, order[i]);
    }
}

/**
 * @brief Unconditionally flip facing and mirror all six joint X positions and rotations.
 */
void Stolas_TurnAround(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts;
    s32 halfTurn;
    s32 i;

    entity->flipX ^= 1;
    halfTurn = 0x20000;
    parts = entity->ext.joint.parts;
    for (i = STOLAS_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = -parts->offsetX;
        parts->rotationAngle = halfTurn - parts->rotationAngle;
    }
}

/**
 * @brief Clear the selected joint target and current hanging-angle fields.
 */
void Stolas_ClearJointAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief Save X/Y positions of all six joints into their backup fields.
 */
void Stolas_SaveJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    s32 i;

    for (i = STOLAS_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief Restore X/Y positions of all six joints from their backup fields.
 */
void Stolas_RestoreJointPositions(struct EwramData_EntityData *entity)
{
    struct EntityJointPart *parts = entity->ext.joint.parts;
    s32 i;

    for (i = STOLAS_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/**
 * @brief Apply player contact damage at the entity position; effect field is 1 for attack index 2 and zero otherwise, with reaction field 1.
 */
void Stolas_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    if ((s8)entity->ext.jointPart.index == 2)
        contactDamage.attributes = 1;
    else
        contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
