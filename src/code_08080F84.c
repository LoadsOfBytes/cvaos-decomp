/* Mirror joint positions and selected joint angles when facing changes. */
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
/**
 * @brief Face the direction given by Entity_IsTargetRight; when turning around
 * mirror the x offsets of all 11 parts and the angle unk_8 of parts
 * 1, 3, 4, 5, 8 and 10.
 */
void Golem_FaceTarget(struct EwramData_EntityData *entity)
{
    s32 facing = entity->flipX;
    if (facing != Entity_IsTargetRight(entity))
    {
        struct GolemPart *base;
        s32 index;
        entity->flipX ^= 1;
        base = entity->ext.golem.parts;
        for (index = 0; index < 11; index++)
        {
            base[index].offsetX = -base[index].offsetX;
            if (index == 1 || index == 3 || index == 4 || index == 5 || index == 8 || index == 10)
                base[index].rotationAngle = 0x20000 - base[index].rotationAngle;
        }
    }
}
