#include "structs/ewram.h"

/**
 * @brief Toggle the stationary beam sprite affine mode from owner scale;
 * request deletion when the owner beam flag clears or the owner is hidden.
 * Do not free the shared owner affine slot.
 */
void BeamSkeletonBeamSprite_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->ext.beamSkeletonBeam.ownerIndex];
    if ((u16)owner->ext.beamSkeleton.beamLevel > 0xFFFE)
    {
        entity->affine = 0;
        ((u8 *)entity)[0x59] &= 0x7F;
    }
    else
    {
        entity->affine = 1;
        ((u8 *)entity)[0x59] |= 0x80;
    }
    if (!(owner->ext.beamSkeleton.firing & 1) || owner->hidden)
        entity->deleteRequested = 1;
}
