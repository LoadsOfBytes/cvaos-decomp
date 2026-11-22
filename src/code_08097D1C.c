/* Mirror all thirteen joints when the facing direction changes.
 * Angles of joints 0..2, 8 and 11 also reflect around 0x20000. */
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
/**
 * @brief When target-facing changes, flip and mirror all thirteen joint X positions; reflect rotations for joints 0..2, 8 and 11.
 */
void CreakingSkull_FaceTarget(struct EwramData_EntityData *entity)
{
    s32 facing = entity->flipX;
    if (facing != Entity_IsTargetRight(entity))
    {
        struct EntityJointPart *p;
        struct EntityJointPart *parts;
        s32 turn;
        entity->flipX ^= 1;
        turn = 0x20000;
        parts = entity->unk_500.jointParts;
        p = parts;
        do
        {
            p->offsetX = -p->offsetX;
            if (p <= parts + 2 || p == parts + 8 || p == parts + 11)
                p->rotationAngle = turn - p->rotationAngle;
            p++;
        /* The original loop tests the addresses with a signed branch. */
        } while ((s32)p <= (s32)(parts + 12));
    }
}
