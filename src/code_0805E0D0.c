/* EventScene03AttachedEffect_Update. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);

/**
 * @brief 5E0D0 | Keep an attached effect on body part 0 of its parent
 * entityData[unk_4F5] (Sprite_GetPartCornerOffset), with the parent's facing and
 * palette; deleted (deleteRequested) once the parent slot holds another object
 * (unk_51A differs).
 */
void EventScene03AttachedEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct { s16 x; s16 y; } point;

    if (parent == NULL || parent->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
    {
        entity->deleteRequested = 1;
        return;
    }
    Sprite_GetPartCornerOffset(parent, 0, &point.x);
    entity->posX.part16.integer = parent->posX.part16.integer + point.x;
    entity->posY.part16.integer = parent->posY.part16.integer + point.y;
    entity->flipX = parent->flipX;
    Anim_Update(entity);
    entity->palette = parent->palette;
}
