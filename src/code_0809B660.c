#include "code_08009A0.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);

/**
 * @brief When target-facing changes, flip and mirror all six joint X positions and reflect their rotations around 0x20000.
 */
void Stolas_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 facing = entity->flipX;
    if (facing != Entity_IsTargetRight(entity))
    {
        struct EntityJointPart *part;
        s32 i;
        s32 halfTurn;
        entity->flipX ^= 1;
        halfTurn = 0x20000;
        part = entity->ext.joint.parts;
        for (i = 5; i >= 0; i--)
        {
            part->offsetX = -part->offsetX;
            part->rotationAngle = halfTurn - part->rotationAngle;
            part++;
        }
    }
}
