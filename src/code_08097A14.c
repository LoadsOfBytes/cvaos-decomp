#include <string.h>
#include "joint_entity.h"
#include "code_08009A0.h"
#include "structs/ewram.h"

extern void CreakingSkull_InterpolateJointsInOrder(struct EwramData_EntityData *entity, u8 *order);
extern void CreakingSkull_AttachJointSetAngle(struct EwramData_EntityData *entity, s32 partIndex, s32 anchorIndex, s32 angleSpec, s32 length);
extern const u8 sUnk_08119063[];

/**
 * @brief Bestiary state (3): cycle four poses through the 13-joint chain
 * with the stored timers, then reset the joint angles and repeat.
 */
void CreakingSkull_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    u8 parts[13];

    memcpy(parts, sUnk_08119063, sizeof(parts));
    switch (entity->subState)
    {
    case 0:
        entity->subState = 20;
    case 20:
        entity->timer = 14;
        JOINT_POSE(entity) = 1;
        entity->subState++;
    case 21:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0x18;
        JOINT_POSE(entity) = 2;
        break;
    case 22:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 14;
        JOINT_POSE(entity) = 3;
        break;
    case 23:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        entity->subState++;
        entity->timer = 0x18;
        JOINT_POSE(entity) = 0;
        break;
    case 24:
        CreakingSkull_InterpolateJointsInOrder(entity, parts);
        if ((s8)--entity->timer > 0)
            break;
        CreakingSkull_AttachJointSetAngle(entity, 12, 11, 0x14000, 0x1F0000);
        CreakingSkull_AttachJointSetAngle(entity, 1, 0, 0x17800, 0x1E0000);
        CreakingSkull_AttachJointSetAngle(entity, 2, 1, 0x1E000, 0x1C0000);
        entity->subState = 21;
        entity->timer = 14;
        JOINT_POSE(entity) = 1;
        break;
    }
}
