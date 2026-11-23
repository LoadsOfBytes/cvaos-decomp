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
        if(entity->ext.grahamShot.homing)
        {
            struct EwramData_EntityData *player=gEwramData->playerState.player;
            entity->ext.grahamShot.angle=Entity_StepAngleTowardPoint(entity,player->posX.whole,player->posY.whole-0x100000,
                                                entity->ext.grahamShot.angle,entity->ext.grahamShot.turn);
        }
        entity->ext.grahamShot.turn+=42;
        if(entity->ext.grahamShot.turn>0x4000)
            entity->ext.grahamShot.turn=0x4000;
        entity->ext.grahamShot.speed+=0x1800;
        if(entity->ext.grahamShot.speed>0x80000)
            entity->ext.grahamShot.speed=0x80000;
        { s32 sine=Sin(entity->ext.grahamShot.angle+0x4000);
            s32 speed=entity->ext.grahamShot.speed;
            entity->velX.whole=(sine>>8)*(speed>>8); }
        { s32 sine=Sin(entity->ext.grahamShot.angle);
            s32 speed=entity->ext.grahamShot.speed;
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
