/* Alastor bestiary preview, target-box tests and afterimage lifetime. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);

/**
 * @brief Bestiary state (8): move the displayed entity up 16 pixels once.
 */
void Alastor_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        entity->posY.part16.integer -= 0x10;
        entity->subState++;
    }
}

/**
 * @brief Test the current target in a supplied-size rectangle centered
 * horizontally on the owner and starting 40 pixels below it.
 */
s32 Alastor_IsTargetInLowerBox(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    u16 left = entity->posX.part16.integer - (width >> 1);
    u16 top = entity->posY.part16.integer + 0x28;

    return Entity_IsInRect(target, left, top, width, height);
}

/**
 * @brief Test the current target in a supplied-size rectangle centered
 * horizontally on the owner and ending at its Y coordinate.
 */
s32 Alastor_IsTargetInUpperBox(struct EwramData_EntityData *entity, u16 width, u16 height)
{
    struct EwramData_EntityData *target = gEwramData->playerState.enemyTarget;
    u16 left = entity->posX.part16.integer - (width >> 1);
    u16 top = entity->posY.part16.integer - height;

    return Entity_IsInRect(target, left, top, width, height);
}

/**
 * @brief On timer underflow or owner removal, free the affine slot and
 * delete the copied sprite.
 */
void AlastorAfterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];

    if (--entity->timer == 0xFF || owner->updateFunc == NULL)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
