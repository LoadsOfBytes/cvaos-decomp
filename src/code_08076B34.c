/* Segmented boss head: aim at the target point while the body history shifts,
 * charge, then fire a straight (10-12) or sweeping (20-22) volley. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "syscalls.h"
#include "structs/ewram.h"
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern void WhiteDragon_SpawnSpark(struct EwramData_EntityData *,s32,s32,s32);
extern void WhiteDragon_SpawnChargeParticle(struct EwramData_EntityData *,s32,s32);
extern void WhiteDragon_SpawnBreath(struct EwramData_EntityData *);

#define RADIUS(e) ((e)->velY.part16.integer)
#define SPIN_SPEED(e) ((e)->velY.part16.fraction)
#define SPIN(e) ((e)->velX.part16.fraction)
#define HEAD_ANGLE(e) ((e)->velX.part16.integer)
#define SWAY(e) (e->ext.whiteDragon.sway)
#define SWAY_PHASE(e) (e->ext.whiteDragon.swayPhase)
#define SWEEP_DIR(e) ((e)->accelX.part16.fraction)
#define SHOT_ANGLE(e) ((e)->accelX.part16.integer)

/**
 * @brief 76B34 | Head attack state machine (subState).
 *
 * Notes for matching: `dx` of state 21 lives in one basic block across two
 * calls, so local-alloc gives it r4 and the entity pointer gets r5; it is
 * narrowed in place (`dx <<= 16; dx >>= 16`).  `angle`/`target` are shared
 * function-level variables and state 21 uses them the other way round
 * (the new direction in `angle`), which fixes r1/r2 there.
 */
void WhiteDragon_UpdateAttack(register struct EwramData_EntityData *entity)
{
    u16 *history=(u16 *)entity->ext.whiteDragon.history;
    s32 i;
    u32 angle,target;
    switch(entity->subState)
    {
    case 0:
    {
        s32 changing=0;
        s32 dx,dy;
        if(RADIUS(entity)>32)
        { RADIUS(entity)--; changing=1; }
        if((u16)SPIN_SPEED(entity)<0x800)
        { SPIN_SPEED(entity)=0x800; changing++; }
        SPIN(entity)+=SPIN_SPEED(entity);
        for(i=99;i>0;i--) history[i]=history[i-1];
        history[0]=SPIN(entity);
        if(SWAY(entity)>0)
        {
            SWAY(entity)-=2;
            changing++;
            if(SWAY(entity)<0) SWAY(entity)=0;
        }
        if((u16)SWAY_PHASE(entity)>0x8000)
        {
            if((u16)SWAY_PHASE(entity)<0xFEB0)
            { SWAY_PHASE(entity)+=320; changing++; }
        }
        else if((u16)SWAY_PHASE(entity)<0x7EB0)
        { SWAY_PHASE(entity)+=320; changing++; }
        dx=entity->ext.whiteDragonAim.x-(entity->ext.whiteDragon.anchorX+(s16)entity->ext.whiteDragon.offsetX);
        dy=entity->ext.whiteDragonAim.y-(entity->ext.whiteDragon.anchorY+entity->ext.whiteDragon.offsetY);
        target=ArcTan2(dx,dy);
        if(entity->flipX)
        {
            angle=(u16)HEAD_ANGLE(entity);
            if(angle<0x8000) angle+=0x10000;
            if(target<0x8000) target+=0x10000;
            if(angle<target)
            { changing++; angle+=128; if(angle>target) angle=target; goto aimed; }
        }
        else
        {
            angle=(u16)HEAD_ANGLE(entity);
            if(angle<target)
            { changing++; angle+=128; if(angle>target) angle=target; goto aimed; }
        }
        if(angle>target)
        { changing++; angle-=128; if(angle<target) angle=target; }
aimed:
        *(u16 *)&HEAD_ANGLE(entity)=angle;
        if(entity->timer<40) entity->timer++;
        else if(changing==0)
        { entity->subState++; entity->timer=changing; PlaySong(0x87); }
        break;
    }
    case 1:
        if(RADIUS(entity)<100) RADIUS(entity)+=4;
        if(entity->timer++>39)
        {
            entity->state=0; entity->subState=0;
            goto cooldown;
        }
        break;
    case 10: case 20:
        if(!entity->timer) { entity->animFrame=1; PlaySong(0x77); }
        if(entity->timer<30)
        {
            if(!(entity->timer&7)) WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer+1);
        }
        else if(!(entity->timer&3)) WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer+1);
        if(entity->subState==20 && (entity->timer&7)==2)
            WhiteDragon_SpawnChargeParticle(entity,entity->posX.whole,entity->posY.whole);
        if(entity->timer++>59) { entity->subState++; entity->timer=0; }
        break;
    case 11: case 21:
        if(!entity->timer) { entity->animFrame=2; PlaySong(0x77); }
        if(++entity->timer)
        {
            entity->subState++; entity->timer=0;
            if(entity->subState==22)
            {
                s32 dx,dy;
                dx=Target_GetPixelX(entity);
                dx-=entity->posX.part16.integer;
                dx<<=16; dx>>=16;
                dy=Target_GetPixelY(entity);
                dy-=({ s32 t_=(u16)entity->posY.part16.integer; t_+=20; t_; });
                angle=(u16)ArcTan2(dx,(s16)dy);
                target=(u16)SHOT_ANGLE(entity);
                if(angle<0x4000) angle+=0x10000;
                if(target<0x4000) target+=0x10000;
                if(angle>target) SWEEP_DIR(entity)=1;
                else SWEEP_DIR(entity)=0;
            }
        }
        break;
    case 12: case 22:
        if(entity->timer++>59)
        {
            entity->state=0; entity->subState=0; entity->animFrame=0;
 cooldown:
            entity->ext.whiteDragon.moveTimer=RandomNumberGenerator()%60+60;
            entity->ext.whiteDragon.shotTimer=RandomNumberGenerator()%60;
        }
        else if(entity->timer<30 && (entity->timer&3)==1)
        {
            WhiteDragon_SpawnBreath(entity); PlaySong(0x77);
            if(entity->subState==22)
            {
                if(*(u16 *)&SWEEP_DIR(entity)) SHOT_ANGLE(entity)+=0x300;
                else SHOT_ANGLE(entity)-=0x300;
            }
        }
        break;
    }
}
