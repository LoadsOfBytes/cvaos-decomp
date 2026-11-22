/* Retreat from a hit, turn back toward the target and resume flight. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
extern void EntityMove(struct EwramData_EntityData *);
/**
 * @brief Recoil state machine (state 3): fly up and away from the target, slowing
 * down, then resume.
 */
void Harpy_UpdateRecoil(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            entity->flipX=Entity_IsTargetRight(entity)^1;
            if(entity->flipX)
                entity->velX.whole=0x30000;
            else
                entity->velX.whole=-0x30000;
            entity->velY.whole=-0x30000;
            entity->subState++;
            break;
        case 1:
            if(entity->velY.whole>-0x14000)
            {
                entity->flipX=Entity_IsTargetRight(entity);
                Anim_StartIndexed(entity,1,0,1);
                entity->subState++;
            }
        case 2:
            EntityDampVelocityX(entity,12);
            EntityDampVelocityY(entity,12);
            EntityMove(entity);
            if(entity->velY.whole>-0x2000)
            {
                entity->state=0;
                entity->subState=10;
                entity->stage=0;
                Anim_StartIndexed(entity,0,0,1);
            }
            break;
    }
}
