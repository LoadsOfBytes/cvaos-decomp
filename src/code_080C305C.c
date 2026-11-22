/* Alternate movement helper reacts to the player's distance and random pauses. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
static inline s32 HorizontalDistance(struct EwramData_EntityData *entity)
{
    s32 value=(s16)Entity_GetTargetDistanceX(entity);
    if(value<0) value=-value;
    return value;
}
/**
 * @brief Unreferenced movement state between the Ghost Dancer functions:
 * reverse the velocity, then react when the target is above within 160
 * px and horizontally close.
 */
void GhostDancer_UpdateUnusedState(struct EwramData_EntityData *entity)
{
    switch(entity->subState)
    {
        case 0:
            if(!entity->flipX)
                entity->velX.whole=-entity->velX.whole;
            entity->velX.whole=-entity->velX.whole;
            Anim_StartIndexed(entity,0,0,1);
            entity->subState++;
            break;
        case 1:
        {
            u8 *animation;
            if((s16)Entity_GetTargetDistanceY(entity)<0 && (s16)Entity_GetTargetDistanceY(entity)>-160 && HorizontalDistance(entity)<=127)
            {
                u32 frame=entity->animScriptIndex;
                animation=&entity->animScriptIndex;
                if(frame!=1) Anim_StartIndexed(entity,1,0,1);
            }
            else
            {
                u32 frame=entity->animScriptIndex;
                animation=&entity->animScriptIndex;
                if(frame!=0) Anim_StartIndexed(entity,0,0,1);
            }
            if((s16)Entity_GetTargetDistanceY(entity)<0 && (s16)Entity_GetTargetDistanceY(entity)>-160 && HorizontalDistance(entity)<=95)
            {
                entity->state=2;
                entity->subState=0;
                break;
            }
            if(!(RandomNumberGenerator() & 127) && *animation==0)
            {
                Anim_StartIndexed(entity,1,0,1);
                entity->subState++;
            }
            break;
        }
        case 2:
            if((*(u32 *)&entity->animFlags & 0xFFFFFF00)==0x100)
            {
                Anim_StartIndexed(entity,0,0,1);
                entity->subState--;
            }
            break;
    }
}
