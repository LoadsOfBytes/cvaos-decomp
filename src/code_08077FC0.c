/* Repeated leaps record motion for the body and turn at ledges. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *,s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *,s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *,s32,s32,s32,s32);
extern void Effect_SpawnRisingParticle(s32,s32,s32,s32);
/**
 * @brief Flight state machine (state 0): fly across the room toward the target,
 * turning at the room edges (unk_514 or 15 px margin) with a wave motion,
 * and record the position history that the body segments follow.
 */
void Quezlcoatl_UpdateFly(struct EwramData_EntityData *entity)
{
    u8 (*history)[2]=(u8 (*)[2])entity->ext.quezlcoatl.history;
    s32 x,y,i,turn,collision;
    switch(entity->subState)
    {
        case 0: entity->subState=1;
        case 1:
            if(!(entity->ext.quezlcoatl.bounceCount&1)) entity->flipX=Entity_IsTargetRight(entity);
            if(!entity->unk_514.part16.spawnParam0) turn=Entity_GetHorizontalRoomEdge(entity,15);
            else turn=Entity_GetHorizontalRoomEdge(entity,entity->unk_514.part16.spawnParam0);
            if(turn==1) { entity->flipX=1; entity->ext.quezlcoatl.bounceCount=0; }
            else if(turn==2) { entity->flipX=0; entity->ext.quezlcoatl.bounceCount=0; }
            if(entity->flipX) entity->velX.whole=0x20000;
            else entity->velX.whole=-0x20000;
            entity->velY.whole=-0x40000;
            entity->accelY.whole=0x2800;
            entity->subState++;
            entity->timer=0;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=0; history[0][1]=0;
            break;
        case 2:
            x=(s16)entity->posX.part16.integer;
            y=entity->posY.part16.integer;
            if((u8)(entity->timer++-19)<=14) EntityDampVelocityX(entity,32);
            collision=Entity_MoveWithCollision(entity,0x10000,-0x100000,0x60000,8);
            x=(s16)entity->posX.part16.integer-x;
            y=entity->posY.part16.integer-y;
            for(i=119;i>0;i--)
            {
                history[i][0]=history[i-1][0];
                history[i][1]=history[i-1][1];
            }
            history[0][0]=x; history[0][1]=y;
            if(collision&4)
            {
                entity->ext.quezlcoatl.bounceCount++;
                entity->subState=1; entity->animFrame=12;
                Effect_SpawnRisingParticle(entity->posX.whole,entity->posY.whole,entity->spriteLayer-1,1);
                if(!EntityIsOutsideScreen(entity,6)) PlaySong(0x7C);
            }
            else
            {
                if((u32)(entity->velY.whole+0x1FFFF)<=0x3FFFE) entity->animFrame=13;
                else entity->animFrame=12;
            }
            break;
    }
}
