/* Ground waiting, animation-timed wingbeats and the transition into flight. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)
static inline s32 TargetDistanceX(struct EwramData_EntityData *entity)
{
    s32 distance=(s16)Entity_GetTargetDistanceX(entity);
    if(distance<0) distance=-distance;
    return distance;
}
/**
 * @brief Approach state machine (state 0): face the target and wait until it is
 * within 89 px horizontally or the Harpy is damaged, then fly around it and
 * start the attack (state 1).
 */
void Harpy_UpdateApproach(struct EwramData_EntityData *entity)
{
    s32 state=entity->subState;
    switch(state)
    {
        case 0:
            if(FRAME_PAIR(entity)==0)
                entity->flipX=Entity_IsTargetRight(entity);
            if(TargetDistanceX(entity)<=89 || Enemy_IsDamaged(entity))
            {
                entity->subState=10;
                entity->stage=state;
                entity->timer=state;
                entity->velX.whole=state;
                entity->velY.whole=state;
                entity->accelX.whole=state;
                entity->accelY.whole=state;
            }
            break;
        case 1:
            entity->flipX=Entity_IsTargetRight(entity);
            EntityDampVelocityX(entity,12);
            EntityAccelerateAndMove(entity,0x80000);
            if(FRAME_PAIR(entity)==1)
            {
                u32 beat=entity->stage;
                if((u8)beat==0)
                {
                    entity->velY.whole=-0x20000;
                    entity->timer++;
                    entity->stage=beat+1;
                    PlaySong(0x8B);
                }
                else if((u8)beat==1)
                {
                    entity->velY.whole-=0x20000;
                    entity->timer++;
                    entity->stage=beat+1;
                    PlaySong(0x8B);
                }
            }
            if(entity->timer>1 && entity->velY.whole>0)
            {
                entity->flipX=Entity_IsTargetRight(entity);
                entity->state=1;
                entity->subState=0;
            }
            break;
        case 10:
        {
            u16 *frame;
            u32 mask=0xFFFF;
            u32 current=(u16)(FRAME_PAIR(entity) & mask);
            frame=(u16 *)&entity->animStep;
            if(!current)
                entity->flipX=Entity_IsTargetRight(entity);
            if((mask &= *frame)==1)
            {
                s32 targetY;
                s16 *height;
                s32 zero,one;
                s32 limit;
                if(entity->flipX)
                    entity->velX.whole=0x14000;
                else
                    entity->velX.whole=-0x14000;
                targetY=(s16)Target_GetPixelY(entity);
                limit=entity->posY.part16.integer;
                limit+=32;
                targetY-=limit;
                height=&entity->posY.part16.integer;
                if(targetY<0)
                    entity->velY.whole=-0x1A000;
                else
                    entity->velY.whole=-0xC000;
                entity->accelY.whole=0x1000;
                zero=0;
                one=1;
                entity->stage=one;
                if(TargetDistanceX(entity)<=80 &&
                   (s16)Target_GetPixelY(entity)-*height>31 &&
                   (s16)Target_GetPixelY(entity)-*height<=64)
                {
                    Anim_StartIndexed(entity,1,0,1);
                    entity->subState=one;
                    entity->stage=zero;
                    entity->timer=zero;
                    entity->velY.whole=-0x20000;
                    entity->accelY.whole=0x1800;
                    return;
                }
                PlaySong(0x8B);
            }
            if(entity->stage)
            {
                if(*(u8 *)frame==1)
                    entity->accelY.whole=0;
                else
                    entity->accelY.whole=0x1000;
            }
            EntityDampVelocityX(entity,32);
            EntityAccelerateAndMove(entity,0x80000);
            break;
        }
    }
}
