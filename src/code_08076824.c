/* Idle oscillation, target detection and selection of the attack sequence. */
#include "code_08009A0.h"
#include "white_dragon_entity.h"
extern u16 ArcTan2(s32 x, s32 y);
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern s32 Target_IsInRect(s16,s16,s16,s16);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
/**
 * @brief Idle state (state 0, also the bestiary preview state 4): sway the head on
 * its neck path, record the position history for the neck segments and
 * start the attack (state 1) when the target is in reach.
 */
void WhiteDragon_UpdateIdle(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    u16 *history=(u16 *)WHITE_DRAGON_HISTORY(entity);
    u32 angle;
    s32 i,timer,x,dx,dy;
    s32 flipped;
    if(entity->subState!=0) return;
    if(entity->velY.part16.integer<72) entity->velY.part16.integer++;
    else if(entity->velY.part16.integer>72) entity->velY.part16.integer--;
    WHITE_DRAGON_SWAY_PHASE(entity)+=320;
    if((u16)entity->velY.part16.fraction<0x400) entity->velY.part16.fraction+=16;
    else if((u16)entity->velY.part16.fraction>0x400) entity->velY.part16.fraction-=16;
    entity->velX.part16.fraction+=entity->velY.part16.fraction;
    for(i=99;i>0;i--) history[i]=history[i-1];
    history[0]=entity->velX.part16.fraction;
    if(WHITE_DRAGON_SWAY(entity)<128) WHITE_DRAGON_SWAY(entity)++;
    else if(WHITE_DRAGON_SWAY(entity)>128) WHITE_DRAGON_SWAY(entity)--;
    if(entity->flipX)
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000) angle+=0x10000;
        if(angle<0x10000) { angle+=64; if(angle<=0x10000) goto store1; }
        else { angle-=64; if(angle>=0x10000) goto store1; }
        angle=0x10000;
    store1:;
    }
    else
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000) { angle+=64; if(angle>0x8000) angle=0x8000; }
        else { angle-=64; if(angle<0x8000) angle=0x8000; }
    }
    entity->velX.part16.integer=angle;
    if(entity->state==4) return;
    { u16 t_ = WHITE_DRAGON_MOVE_TIMER(entity); timer=(s16)WHITE_DRAGON_MOVE_TIMER(entity);
    if((s16)timer) WHITE_DRAGON_MOVE_TIMER(entity)=t_-1;
    else if((RandomNumberGenerator()&255)<=64)
    {
        x=(s16)WHITE_DRAGON_OFFSET_X(entity)+(WHITE_DRAGON_ANCHOR_X(entity)-(s16)camera->xPos.part16.integer);
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped)
        {
            if(Target_IsInRect(x+40,entity->posY.part16.integer-100,84,200))
            {
                entity->state=1;
                entity->subState=timer;
                entity->timer=timer;
                { s32 v_ = Target_GetPixelX(entity); { u32 b_ = camera->xPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_X(entity) = v_; }
                { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_Y(entity) = v_ - 20; } /* FAKEMATCH: pinned register */
                return;
            }
        }
        else if(Target_IsInRect(x-124,entity->posY.part16.integer-100,84,200))
        {
            entity->state=1;
            entity->subState=flipped;
            entity->timer=flipped;
            { s32 v_ = Target_GetPixelX(entity); { register u32 b_ asm("r2") = camera->xPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_X(entity) = v_; } /* FAKEMATCH: pinned register */
            { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_Y(entity) = v_ - 20; } /* FAKEMATCH: pinned register */
            return;
        }
    }
    else WHITE_DRAGON_MOVE_TIMER(entity)=RandomNumberGenerator()%30;
    }
    { register u32 z_ asm("r2"); asm("" : "=r"(z_)); if(WHITE_DRAGON_SHOT_TIMER(entity)) { asm("" :: "r"(z_)); WHITE_DRAGON_SHOT_TIMER(entity)--; } /* FAKEMATCH: fake-live register steers reload rotation */
    else if((RandomNumberGenerator()&255)<=64)
    {
        x=(s16)entity->posX.part16.integer; dy=(s16)entity->posY.part16.integer;
        if(entity->flipX)
        {
            if(!Target_IsInRect(x,entity->posY.part16.integer-100,80,200)) return;
            entity->state=1;
            if(RandomNumberGenerator()&1) entity->subState=20;
            else entity->subState=10;
            entity->timer=0;
            { s32 v_ = Target_GetPixelX(entity); { u32 b_ = camera->xPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_X(entity) = v_; }
            { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_Y(entity) = v_ - 20; } /* FAKEMATCH: pinned register */
            dx=Target_GetPixelX(entity); { u16 px_; register u32 z_ asm("r1"); asm("" : "=r"(z_)); px_ = entity->posX.part16.integer; asm("" :: "r"(z_)); dx-=px_; } /* FAKEMATCH: fake-live register steers reload rotation */
        }
        else
        {
            if(!Target_IsInRect(x-80,entity->posY.part16.integer-100,80,200)) return;
            entity->state=1;
            if(RandomNumberGenerator()&1) entity->subState=20;
            else entity->subState=10;
            entity->timer=0;
            { s32 v_ = Target_GetPixelX(entity); { register u32 b_ asm("r3") = camera->xPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_X(entity) = v_; } /* FAKEMATCH: pinned register */
            { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } WHITE_DRAGON_AIM_Y(entity) = v_ - 20; } /* FAKEMATCH: pinned register */
            dx=Target_GetPixelX(entity); { u16 px_; register u32 z_ asm("r3"); asm("" : "=r"(z_)); px_ = entity->posX.part16.integer; asm("" :: "r"(z_)); dx-=px_; } /* FAKEMATCH: fake-live register steers reload rotation */
        }
        dx<<=16; dx>>=16;
        dy=Target_GetPixelY(entity)-({ u32 t_ = (u16)entity->posY.part16.integer+20; t_; }); dy<<=16; dy>>=16; entity->accelX.part16.integer=ArcTan2(dx,dy);
    }
    else WHITE_DRAGON_SHOT_TIMER(entity)=RandomNumberGenerator()%30+30; }
}
