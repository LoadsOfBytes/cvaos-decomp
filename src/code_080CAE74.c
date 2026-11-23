/* Beam Skeleton moving beam and sound maintenance. */
#include "structs/ewram.h"
#include "beam_skeleton_entity.h"
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Sound_PlayTracked(s32 id);
extern void Sound_StopTracked(s32 id);
#define ABS(x) ((x) < 0 ? -(x) : (x))

static inline void ResetBeam(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[BEAM_OWNER_INDEX(entity)];
    entity->posX.whole = -0xB0000;
    if (entity->flipX)
        entity->posX.whole = -entity->posX.whole;
    entity->posX.whole += owner->posX.whole;
    entity->animFrame = 22;
    entity->velX.whole = 0xC0000;
    if (!entity->flipX)
    {
        entity->velX.whole = -entity->velX.whole;
        entity->accelX.whole = -entity->accelX.whole;
    }
}

/**
 * @brief Move and reset the contact beam relative to its owner, toggle
 * affine display from owner scale and maintain tracked sound 0x66.
 * Request deletion when the owner beam flag clears or the owner is hidden;
 * the shared affine slot remains owned by the enemy.
 */
void BeamSkeletonBeam_Update(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    struct EwramData_EntityData *owner = &gEwramData->entityData[BEAM_OWNER_INDEX(entity)];
    if ((u16)BEAM_SKELETON_BEAM_LEVEL(owner) > 0x400)
        Hit_CheckWithPlayer(buffer,entity);
    switch (entity->state)
    {
        case 0:
            ResetBeam(entity);
            entity->state++;
        case 1:
            if (--entity->timer == 0xFF)
            {
                entity->timer = 2;
                entity->state++;
            }
            break;
        case 2:
            if ((u16)BEAM_SKELETON_BEAM_LEVEL(owner) > 0xFFFE)
            {
                if (--entity->timer == 0xFF)
                {
                    entity->state++;
                    entity->timer = 1;
                    entity->affine = 1;
                }
            }
            break;
        case 3:
            if (--entity->timer == 0xFF && entity->animFrame == 22)
                entity->animFrame = 23;
            entity->posX.whole += entity->velX.whole;
            entity->velX.whole += entity->accelX.whole;
            if (ABS((s16)entity->posX.part16.integer - (s16)owner->posX.part16.integer) > 92)
            {
                ResetBeam(entity);
                entity->timer = 1;
            }
            break;
    }
    if ((u16)BEAM_SKELETON_BEAM_LEVEL(owner) == 0xFFFF)
    {
        entity->affine = 0;
        ((u8 *)entity)[0x59] &= 0x7F;
    }
    else
    {
        entity->hit.flags |= 2;
        entity->affine = 1;
        ((u8 *)entity)[0x59] |= 0x80;
    }
    if (!(BEAM_SKELETON_FIRING(owner) & 1) || owner->hidden)
    {
        Sound_StopTracked(0x66);
        entity->deleteRequested = 1;
    }
    else
    {
        if (BEAM_COUNTER(entity) & 3)
        {
            u16 *sounds = gEwramData->trackedSoundIds;
            s32 i;
            for (i=0;i<4;i++,sounds++)
                if (*sounds == 0x66)
                    break;
            if (i==4)
                Sound_PlayTracked(0x66);
        }
        BEAM_COUNTER(entity)++;
    }
}
