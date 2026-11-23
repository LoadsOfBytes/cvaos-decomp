/* GreatArmor_FaceTarget (Great Armor turning around). */
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);

/**
 * @brief 8DD38 | Face the direction given by Entity_IsTargetRight; when turning
 * around mirror the x offsets of all 14 parts and the angle unk_8 of parts
 * 2, 5, 8, 11, 12 and 13.
 */
void GreatArmor_FaceTarget(struct EwramData_EntityData *entity)
{
    u32 flipX = entity->flipX;
    struct GreatArmorPart *parts;
    s32 i;

    if (flipX != Entity_IsTargetRight(entity))
    {
        entity->flipX ^= 1;
        parts = entity->ext.greatArmor.parts;
        for (i = 0; i < 14; i++)
        {
            parts[i].offsetX = -parts[i].offsetX;
            if (i == 2 || i == 5 || i == 8 || i == 11 || i == 12 || i == 13)
                parts[i].rotationAngle = 0x20000 - parts[i].rotationAngle;
        }
    }
}
