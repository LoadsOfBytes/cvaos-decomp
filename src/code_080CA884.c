/* Beam Skeleton approach and attack selection. */
#include "code_08009A0.h"
#include "beam_skeleton_entity.h"
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
            BEAM_SKELETON_COOLDOWN(entity) = RandomNumberGenerator() % 32;
            entity->subState++;
            entity->stage = 0;
        }
        case 1:
            Entity_FaceTarget(entity);
            entity->timer = 20;
            BEAM_SKELETON_WALK_SPEED(entity) = RandomNumberGenerator() % 0x4000 + 0x8000;
            if (Entity_IsTargetInCenteredBox(entity,RandomNumberGenerator() % 128 + 64,128))
                BEAM_SKELETON_WALK_SPEED(entity) = -BEAM_SKELETON_WALK_SPEED(entity);
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            entity->stage = 0;
        case 2:
            if (BEAM_SKELETON_COOLDOWN(entity) != 0)
                BEAM_SKELETON_COOLDOWN(entity)--;
            if (BEAM_SKELETON_WALK_SPEED(entity) > 0)
                BEAM_SKELETON_WALK_SPEED(entity) -= BEAM_SKELETON_WALK_SPEED(entity) >> 4;
            else
                BEAM_SKELETON_WALK_SPEED(entity) += ABS(BEAM_SKELETON_WALK_SPEED(entity)) >> 4;
            if (entity->timer == 0)
            {
                if (BEAM_SKELETON_COOLDOWN(entity) == 0 && Entity_IsTargetInCenteredBox(entity,100,40))
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                    return;
                }
                BEAM_SKELETON_FIRE_URGE(entity)++;
                if (RandomNumberGenerator() % 40 < BEAM_SKELETON_FIRE_URGE(entity))
                {
                    s32 distance = (s16)Entity_GetTargetDistanceX(entity);
                    if (ABS(distance) > 48)
                    {
                        BEAM_SKELETON_FIRE_URGE(entity) = 0;
                        if (RandomNumberGenerator() % 50 <= 14)
                        {
                            entity->state = 2;
                            entity->stage = 0;
                            entity->subState = 0;
                        }
                        return;
                    }
                    BEAM_SKELETON_FIRE_URGE(entity) >>= 1;
                }
                entity->subState--;
                entity->stage = 0;
            }
            else
                entity->timer--;
            Entity_Walk(entity,BEAM_SKELETON_WALK_SPEED(entity),sBeamSkeletonWalkProfile,12);
            break;
    }
}
