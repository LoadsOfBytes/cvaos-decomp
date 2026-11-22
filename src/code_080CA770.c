/* Beam Skeleton patrol and selection of the attack state. */
#include "code_08009A0.h"
#include "beam_skeleton_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity,s32 mode);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity,s32 width,s32 height);
extern s16 sBeamSkeletonWalkProfile[];

/**
 * @brief Patrol state (0): walk within the stored range, turn at terrain
 * or room bounds and enter approach state 1 near the target.
 */
void BeamSkeleton_UpdatePatrol(struct EwramData_EntityData *entity)
{
    s32 blocked = 0;
    u16 left = BEAM_SKELETON_PATROL_LEFT(entity);
    u16 right = BEAM_SKELETON_PATROL_RIGHT(entity);

    switch (entity->subState)
    {
        case 0:
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity,1,0,1);
            entity->subState++;
            break;
        case 1:
        {
            u16 distance = BEAM_SKELETON_HOME_X(entity) - GetEntityRoomXPositionInteger(entity);
            if (entity->flipX)
            {
                if ((s16)right < GetEntityRoomXPositionInteger(entity) || (s16)distance < -40)
                    entity->flipX = 0;
            }
            else
            {
                if ((s16)left > GetEntityRoomXPositionInteger(entity) || (s16)distance > 40)
                    entity->flipX = 1;
            }
            if (Entity_Walk(entity,0x6000,sBeamSkeletonWalkProfile,12) & 3)
                blocked = 1;
            if (Entity_ClampXToRoom(entity,1))
                blocked = 1;
            if (blocked)
                entity->flipX = 1 - entity->flipX;
            if (Target_IsNearEntity(entity,192,128))
            {
                entity->velX.whole = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
    }
}
