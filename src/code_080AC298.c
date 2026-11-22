/* BigGolem_FaceTarget (Big Golem turning around). */
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);

/**
 * @brief AC298 | Face the direction given by Entity_IsTargetRight; when turning
 * around mirror the x offsets of all 14 parts and the angle unk_8 of parts
 * 2, 4, 5, 8, 12 and 13.
 */
void BigGolem_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 flipX = entity->flipX;
    struct BigGolemPart *parts;
    s32 i;

    if (flipX != Entity_IsTargetRight(entity))
    {
        entity->flipX ^= 1;
        parts = entity->unk_500.bigGolemParts;
        for (i = 0; i < 14; i++)
        {
            parts[i].offsetX = -parts[i].offsetX;
            if (i == 2 || i == 4 || i == 5 || i == 8 || i == 12 || i == 13)
                parts[i].rotationAngle = 0x20000 - parts[i].rotationAngle;
        }
    }
}
