/* Beam Skeleton approach and attack selection. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *entity,u16 width,u16 height);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern s16 sBeamSkeletonWalkProfile[];
#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Approach state (1): vary target-relative movement and wait, then
 * select firing state 3 when aligned or idle state 2 from distance
 * and randomness. Return to patrol when the vertical distance is too large.
 */
void BeamSkeleton_UpdateApproach(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
        {
            s32 distance = (s16)Entity_GetTargetDistanceY(entity);
            if (ABS(distance) > 32)
            {
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
                return;
            }
            entity->ext.beamSkeleton.cooldown = RandomNumberGenerator() % 32;
            entity->subState++;
            entity->stage = 0;
        }
        case 1:
            Entity_FaceTarget(entity);
            entity->timer = 20;
            entity->ext.beamSkeleton.walkSpeed = RandomNumberGenerator() % 0x4000 + 0x8000;
            if (Entity_IsTargetInCenteredBox(entity,RandomNumberGenerator() % 128 + 64,128))
                entity->ext.beamSkeleton.walkSpeed = -entity->ext.beamSkeleton.walkSpeed;
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            entity->stage = 0;
        case 2:
            if (entity->ext.beamSkeleton.cooldown != 0)
                entity->ext.beamSkeleton.cooldown--;
            if (entity->ext.beamSkeleton.walkSpeed > 0)
                entity->ext.beamSkeleton.walkSpeed -= entity->ext.beamSkeleton.walkSpeed >> 4;
            else
                entity->ext.beamSkeleton.walkSpeed += ABS(entity->ext.beamSkeleton.walkSpeed) >> 4;
            if (entity->timer == 0)
            {
                if (entity->ext.beamSkeleton.cooldown == 0 && Entity_IsTargetInCenteredBox(entity,100,40))
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                    return;
                }
                entity->ext.beamSkeleton.fireUrge++;
                if (RandomNumberGenerator() % 40 < entity->ext.beamSkeleton.fireUrge)
                {
                    s32 distance = (s16)Entity_GetTargetDistanceX(entity);
                    if (ABS(distance) > 48)
                    {
                        entity->ext.beamSkeleton.fireUrge = 0;
                        if (RandomNumberGenerator() % 50 <= 14)
                        {
                            entity->state = 2;
                            entity->stage = 0;
                            entity->subState = 0;
                        }
                        return;
                    }
                    entity->ext.beamSkeleton.fireUrge >>= 1;
                }
                entity->subState--;
                entity->stage = 0;
            }
            else
                entity->timer--;
            Entity_Walk(entity,entity->ext.beamSkeleton.walkSpeed,sBeamSkeletonWalkProfile,12);
            break;
    }
}
