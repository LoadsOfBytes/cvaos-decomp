/* Homing turn rate and speed ramp, followed by a timed projectile lifetime. */
#include "code_08009A0.h"
#include "graham_projectile.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_StepAngleTowardPoint(struct EwramData_EntityData *,s32,s32,s32,s32);
extern void Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
/**
 * @brief Ramp turn rate and speed, optionally steer toward the player,
 * move and check contact. Request deletion after the configured
 * lifetime and final eight-update countdown.
 */
void GrahamSceneProjectile_Update(struct EwramData_EntityData *entity)
{
    s32 bounds[4];
    if(entity->state<=1)
    {
        if(GRAHAM_SHOT_HOMING(entity))
        {
            struct EwramData_EntityData *player=gEwramData->playerState.player;
            GRAHAM_SHOT_ANGLE(entity)=Entity_StepAngleTowardPoint(entity,player->posX.whole,player->posY.whole-0x100000,
                                                GRAHAM_SHOT_ANGLE(entity),GRAHAM_SHOT_TURN(entity));
        }
        GRAHAM_SHOT_TURN(entity)+=42;
        if(GRAHAM_SHOT_TURN(entity)>0x4000)
            GRAHAM_SHOT_TURN(entity)=0x4000;
        GRAHAM_SHOT_SPEED(entity)+=0x1800;
        if(GRAHAM_SHOT_SPEED(entity)>0x80000)
            GRAHAM_SHOT_SPEED(entity)=0x80000;
        { s32 sine=Sin(GRAHAM_SHOT_ANGLE(entity)+0x4000);
            s32 speed=GRAHAM_SHOT_SPEED(entity);
            entity->velX.whole=(sine>>8)*(speed>>8); }
        { s32 sine=Sin(GRAHAM_SHOT_ANGLE(entity));
            s32 speed=GRAHAM_SHOT_SPEED(entity);
            entity->velY.whole=(sine>>8)*(speed>>8); }
        entity->posX.whole+=entity->velX.whole;
        entity->posY.whole+=entity->velY.whole;
    }
    Hit_CheckWithPlayer(bounds,entity);
    switch(entity->state)
    {
        case 0:
            if(--entity->timer==0)
            {
                entity->timer=8;
                entity->state++;
            }
            break;
        case 1:
            if(--entity->timer==0)
                entity->deleteRequested=1;
            break;
        default:
            entity->timer++;
            break;
    }
    if(entity->stage)
        Anim_Update(entity);
}
