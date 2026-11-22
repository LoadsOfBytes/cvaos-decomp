#include "structs/ewram.h"
#include "beam_skeleton_entity.h"

/**
 * @brief Toggle the stationary beam sprite affine mode from owner scale;
 * request deletion when the owner beam flag clears or the owner is hidden.
 * Do not free the shared owner affine slot.
 */
void BeamSkeletonBeamSprite_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[BEAM_OWNER_INDEX(entity)];
    if ((u16)BEAM_SKELETON_BEAM_LEVEL(owner) > 0xFFFE)
    {
        entity->affine = 0;
        ((u8 *)entity)[0x59] &= 0x7F;
    }
    else
    {
        entity->affine = 1;
        ((u8 *)entity)[0x59] |= 0x80;
    }
    if (!(BEAM_SKELETON_FIRING(owner) & 1) || owner->hidden)
        entity->deleteRequested = 1;
}
