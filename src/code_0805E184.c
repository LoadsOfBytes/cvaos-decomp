/* Child entity that follows its parent with an offset (EventScene03AttachedHelper_Update).
 * The do/while (0) adds weight to entity so global-alloc gives it r5. */
/* Like EventScene03AttachedEffect_Update, but with an extra (+-8, -8) offset mirrored by the
 * parent's facing. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);

/**
 * @brief Keep the helper on body part 0 of its parent (offset 8/-8 by facing)
 * with the parent palette; delete it when the parent slot holds another
 * object.
 */
void EventScene03AttachedHelper_Update(struct EwramData_EntityData *entity)
{
    struct { s16 x; s16 y; } point;
    s32 dx, dy;
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    dx = 8;
    dy = -8;

    if (parent == NULL || parent->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
    {
        entity->deleteRequested = 1;
        return;
    }
    entity->flipX = parent->flipX;
    Sprite_GetPartCornerOffset(parent, 0, (s16 *)&point);
    if (entity->flipX)
        dx = dy;
    entity->posX.part16.integer = parent->posX.part16.integer + point.x + dx;
    entity->posY.part16.integer = parent->posY.part16.integer + point.y + dy;
    do { Anim_Update(entity); } while (0);
    entity->palette = parent->palette;
}
