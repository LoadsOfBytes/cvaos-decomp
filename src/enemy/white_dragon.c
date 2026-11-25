/*
 * White Dragon (boss): body, segments, breath, sparks, charge particles and death pieces.
 */
#include "core.h"
#include "sound.h"
#include "player_core.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "syscalls.h"
#include "structs/entity_render_flags.h"
#include "graphics.h"
#include "gameplay.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_hit.h"


void WhiteDragon_UpdateIdle(struct EwramData_EntityData *);
void WhiteDragon_UpdateAttack(struct EwramData_EntityData *);
void WhiteDragon_UpdateRecoil(struct EwramData_EntityData *);
void WhiteDragon_UpdateDeath(struct EwramData_EntityData *);
void WhiteDragon_SpawnDeathPiece(struct EwramData_EntityData *,s32,s32,s32);
void WhiteDragon_SpawnSpark(struct EwramData_EntityData *,s32,s32,s32);
u8 WhiteDragon_LoadGraphics(struct EwramData_EntityData *);
void WhiteDragonSegment_Update(struct EwramData_EntityData *);
void WhiteDragonSegment_Draw(struct EwramData_EntityData *);
void WhiteDragon_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void WhiteDragon_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *info);
void WhiteDragon_SpawnChargeParticle(struct EwramData_EntityData *,s32,s32);
void WhiteDragon_SpawnBreath(struct EwramData_EntityData *);
void WhiteDragonBreath_Update(struct EwramData_EntityData *);
u8 WhiteDragonBreath_LoadGraphics(struct EwramData_EntityData *,struct EwramData_EntityData *);
void WhiteDragonBreath_EnableHitBoxes(struct EwramData_EntityData *);
void WhiteDragonBreath_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void WhiteDragonSpark_Update(struct EwramData_EntityData *);
void WhiteDragonChargeParticle_Update(struct EwramData_EntityData *);
void WhiteDragonPiece_Update(struct EwramData_EntityData *);

/* White Dragon dispatch, collision gating and delayed death cleanup. */
void EnemyWhiteDragonUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if(entity->state!=2)
    {
        if(Entity_IsInRect(entity,-16,0,272,176))
            Player_TrackEntity(entity);
        if(entity->state!=2)
            Enemy_UpdateHitFlash(entity);
    }
    if(Entity_IsTimeStopped(entity) && entity->state!=2)
    {
        Hit_CheckWithPlayer(&result,entity);
        return;
    }
    if(entity->hitFlashTimer && entity->state!=3 && entity->state!=2)
    {
        Hit_CheckWithPlayer(&result,entity);
        return;
    }
    switch(entity->state)
    {
        case 0: WhiteDragon_UpdateIdle(entity); break;
        case 1: WhiteDragon_UpdateAttack(entity); break;
        case 2:
        {
            s32 phase=entity->subState;
            if(!phase)
            {
                WhiteDragon_UpdateDeath(entity);
                if(entity->deleteRequested)
                {
                    s32 offset;
                    u32 flipped;
                    entity->deleteRequested=0;
                    entity->hidden=1;
                    entity->subState=1;
                    entity->timer=phase;
                    flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40;
                    offset=0x80000;
                    if(flipped) offset=-0x80000;
                    PlaySong(0x6B);
                    WhiteDragon_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole,10);
                    WhiteDragon_SpawnDeathPiece(entity,entity->posX.whole+offset,entity->posY.whole,11);
                    WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
                    if(entity->ext.whiteDragon.history)
                        Heap_Free((void *)entity->ext.whiteDragon.history);
                }
            }
            else if(entity->timer++>49)
                entity->deleteRequested=1;
            break;
        }
        case 4: WhiteDragon_UpdateIdle(entity); return;
        case 3: WhiteDragon_UpdateRecoil(entity); break;
    }
    Enemy_CheckHitPlayer(entity);
}

/* Allocate a 100-halfword motion history and link ten body segments. */
static inline void StoreRate(s32 value,u16 *destination) { *destination=value; }
void EnemyWhiteDragonCreate(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    u32 box=0x0C10F8F8;
    struct EwramData_EntityData *previous;
    s32 index;
    u16 *angularSpeed;
    u8 *palette;
    u8 demo;
    if(!WhiteDragon_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->animFrame=0;
    entity->ext.whiteDragon.history=(s32)Heap_Alloc(50);
    if(!entity->ext.whiteDragon.history)
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->scrollWithBg=0;
    entity->ext.whiteDragon.offsetX=entity->posX.part16.integer;
    entity->ext.whiteDragon.offsetY=entity->posY.part16.integer;
    entity->ext.whiteDragon.anchorX=camera->xPos.part16.integer;
    entity->ext.whiteDragon.anchorY=camera->yPos.part16.integer;
    if(entity->spawnParam0)
        entity->flipX=1;
    else
        entity->flipX=0;
    if(entity->flipX)
        entity->velX.part16.integer=0;
    else
        entity->velX.part16.integer=(s16)0x8000;
    entity->velX.part16.fraction=0;
    entity->velY.part16.integer=0;
    entity->velY.part16.fraction=0;
    entity->ext.whiteDragon.sway=0;
    entity->ext.whiteDragon.swayPhase=0;
    entity->ext.whiteDragon.moveTimer=0;
    entity->ext.whiteDragon.shotTimer=0;
    previous=entity;
    index=9;
    angularSpeed=(u16 *)&entity->velY.part16.fraction;
    palette=&entity->basePalette;
    do
    {
        struct EwramData_EntityData *part=Entity_CreateObject(WhiteDragonSegment_Update);
        s32 graphics;
        if(!part)
        {
            EntityDelete(entity);
            return;
        }
        part->spriteLayer=entity->spriteLayer+1;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(part,sQuezlcoatlWhiteDragonSpriteSheet,graphics,*palette);
        part->enemyId=entity->enemyId;
        part->basePalette=*palette;
        part->scrollWithBg=0;
        part->drawFunc=WhiteDragonSegment_Draw;
        ENTITY_OWNER(part)=entity;
        part->ext.whiteDragonSegment.prev=previous;
        part->ext.whiteDragonSegment.index=index;
        Anim_StartIndexed(part,1,0,1);
        Anim_Update(part);
        previous=part;
        index--;
    } while(index>=0);
    demo=*(u8 *)((u8 *)gEwramData+0x8E) & 0x40;
    if(demo)
    {
        entity->state=4;
        StoreRate(0x400,angularSpeed);
        return;
    }
    Hit_Enable(entity,8,WhiteDragon_DamagePlayerOnContact, WhiteDragon_OnHit);
    HitBox_SetBody(entity,&box);
    HitBox_SetAttack(entity,&box);
    entity->state=demo;
    return;
    EntityDelete(entity);
    return;
    Hit_Enable(entity,8,WhiteDragon_DamagePlayerOnContact, WhiteDragon_OnHit);
    HitBox_SetBody(entity,&box);
    HitBox_SetAttack(entity,&box);
    entity->state=demo;
}

/* Idle oscillation, target detection and selection of the attack sequence. */
/**
 * @brief Idle state (state 0, also the bestiary preview state 4): sway the head on
 * its neck path, record the position history for the neck segments and
 * start the attack (state 1) when the target is in reach.
 */
void WhiteDragon_UpdateIdle(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    u16 *history=(u16 *)entity->ext.whiteDragon.history;
    u32 angle;
    s32 i,timer,x,dx,dy;
    s32 flipped;
    if(entity->subState!=0) return;
    if(entity->velY.part16.integer<72) entity->velY.part16.integer++;
    else if(entity->velY.part16.integer>72) entity->velY.part16.integer--;
    entity->ext.whiteDragon.swayPhase+=320;
    if((u16)entity->velY.part16.fraction<0x400) entity->velY.part16.fraction+=16;
    else if((u16)entity->velY.part16.fraction>0x400) entity->velY.part16.fraction-=16;
    entity->velX.part16.fraction+=entity->velY.part16.fraction;
    for(i=99;i>0;i--) history[i]=history[i-1];
    history[0]=entity->velX.part16.fraction;
    if(entity->ext.whiteDragon.sway<128) entity->ext.whiteDragon.sway++;
    else if(entity->ext.whiteDragon.sway>128) entity->ext.whiteDragon.sway--;
    if(entity->flipX)
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000) angle+=0x10000;
        if(angle<0x10000 ? (angle+=64)<=0x10000 : (angle-=64)>=0x10000) {} else angle=0x10000;
    }
    else
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000) { angle+=64; if(angle>0x8000) angle=0x8000; }
        else { angle-=64; if(angle<0x8000) angle=0x8000; }
    }
    entity->velX.part16.integer=angle;
    if(entity->state==4) return;
    { u16 t_ = entity->ext.whiteDragon.moveTimer; timer=(s16)entity->ext.whiteDragon.moveTimer;
    if((s16)timer) entity->ext.whiteDragon.moveTimer=t_-1;
    else if((RandomNumberGenerator()&255)<=64)
    {
        x=(s16)entity->ext.whiteDragon.offsetX+(entity->ext.whiteDragon.anchorX-(s16)camera->xPos.part16.integer);
        flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity))&0x40;
        if((u8)flipped)
        {
            if(Target_IsInRect((s16)(x+40),(s16)(entity->posY.part16.integer-100),84,200))
            {
                entity->state=1;
                entity->subState=timer;
                entity->timer=timer;
                { s32 v_ = Target_GetPixelX(entity); { u32 b_ = camera->xPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.x = v_; }
                { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.y = v_ - 20; } /* FAKEMATCH: pinned register */
                return;
            }
        }
        else if(Target_IsInRect((s16)(x-124),(s16)(entity->posY.part16.integer-100),84,200))
        {
            entity->state=1;
            entity->subState=flipped;
            entity->timer=flipped;
            { s32 v_ = Target_GetPixelX(entity); { register u32 b_ asm("r2") = camera->xPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.x = v_; } /* FAKEMATCH: pinned register */
            { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.y = v_ - 20; } /* FAKEMATCH: pinned register */
            return;
        }
    }
    else entity->ext.whiteDragon.moveTimer=RandomNumberGenerator()%30;
    }
    { register u32 z_ asm("r2"); asm("" : "=r"(z_)); if(entity->ext.whiteDragon.shotTimer) { asm("" :: "r"(z_)); entity->ext.whiteDragon.shotTimer--; } /* FAKEMATCH: fake-live register steers reload rotation */
    else if((RandomNumberGenerator()&255)<=64)
    {
        x=(s16)entity->posX.part16.integer; dy=(s16)entity->posY.part16.integer;
        if(entity->flipX)
        {
            if(!Target_IsInRect((s16)x,(s16)(entity->posY.part16.integer-100),80,200)) return;
            entity->state=1;
            if(RandomNumberGenerator()&1) entity->subState=20;
            else entity->subState=10;
            entity->timer=0;
            { s32 v_ = Target_GetPixelX(entity); { u32 b_ = camera->xPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.x = v_; }
            { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.y = v_ - 20; } /* FAKEMATCH: pinned register */
            dx=Target_GetPixelX(entity); { u16 px_; register u32 z_ asm("r1"); asm("" : "=r"(z_)); px_ = entity->posX.part16.integer; asm("" :: "r"(z_)); dx-=px_; } /* FAKEMATCH: fake-live register steers reload rotation */
        }
        else
        {
            if(!Target_IsInRect((s16)(x-80),(s16)(entity->posY.part16.integer-100),80,200)) return;
            entity->state=1;
            if(RandomNumberGenerator()&1) entity->subState=20;
            else entity->subState=10;
            entity->timer=0;
            { s32 v_ = Target_GetPixelX(entity); { register u32 b_ asm("r3") = camera->xPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.x = v_; } /* FAKEMATCH: pinned register */
            { s32 v_ = Target_GetPixelY(entity); { register u32 b_ asm("r7") = camera->yPos.part16.integer; v_ += b_; } entity->ext.whiteDragonAim.y = v_ - 20; } /* FAKEMATCH: pinned register */
            dx=Target_GetPixelX(entity); { u16 px_; register u32 z_ asm("r3"); asm("" : "=r"(z_)); px_ = entity->posX.part16.integer; asm("" :: "r"(z_)); dx-=px_; } /* FAKEMATCH: fake-live register steers reload rotation */
        }
        dx<<=16; dx>>=16;
        dy=Target_GetPixelY(entity)-({ u32 t_ = (u16)entity->posY.part16.integer+20; t_; }); dy<<=16; dy>>=16; entity->accelX.part16.integer=ArcTan2Wide(dx,dy);
    }
    else entity->ext.whiteDragon.shotTimer=RandomNumberGenerator()%30+30; }
}

/* Segmented boss head: aim at the target point while the body history shifts,
 * charge, then fire a straight (10-12) or sweeping (20-22) volley. */

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
        target=ArcTan2Wide((s16)dx,(s16)dy);
        if(entity->flipX)
        {
            angle=(u16)HEAD_ANGLE(entity);
            if(angle<0x8000) angle+=0x10000;
            if(target<0x8000) target+=0x10000;
            if(angle<target)
            { changing++; angle+=128; if(angle>target) angle=target; }
            else if(angle>target)
            { changing++; angle-=128; if(angle<target) angle=target; }
        }
        else
        {
            angle=(u16)HEAD_ANGLE(entity);
            if(angle<target)
            { changing++; angle+=128; if(angle>target) angle=target; }
            else if(angle>target)
            { changing++; angle-=128; if(angle<target) angle=target; }
        }
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
            entity->ext.whiteDragon.moveTimer=RandomNumberGenerator()%60+60;
            entity->ext.whiteDragon.shotTimer=RandomNumberGenerator()%60;
            break;
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
                angle=(u16)ArcTan2Wide((s16)dx,(s16)dy);
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

/* Ease two motion parameters back to rest and leave the reaction state. */
/**
 * @brief Recoil state (state 3): pull the neck in for 30 frames, then back to
 * state 0.
 */
void WhiteDragon_UpdateRecoil(struct EwramData_EntityData *entity)
{
    u16 old=(u16)entity->velY.part16.integer;
    if(entity->velY.part16.integer>48)
        entity->velY.part16.integer=old-2;
    { u16 angle=(u16)entity->ext.whiteDragon.sway;
    if(entity->ext.whiteDragon.sway<=255)
        entity->ext.whiteDragon.sway=angle+2;
    }
    if(entity->timer++>29)
    {
        entity->state=0;
        entity->subState=0;
        entity->timer=0;
        entity->animFrame=0;
    }
}

/* Collapse the body's oscillation while shifting its angle history. */
/**
 * @brief Death motion (state 2): shrink the neck path and let the head sag,
 * throwing sparks (WhiteDragon_SpawnSpark).
 */
void WhiteDragon_UpdateDeath(struct EwramData_EntityData *entity)
{
    u16 *history=(u16 *)entity->ext.whiteDragon.history;
    s32 i;
    u32 angle;
    if(entity->velY.part16.integer<48)
        entity->velY.part16.integer++;
    else if(entity->velY.part16.integer>48)
        entity->velY.part16.integer--;
    if(entity->ext.whiteDragon.sway>256)
        entity->ext.whiteDragon.sway--;
    else if(entity->ext.whiteDragon.sway<256)
        entity->ext.whiteDragon.sway+=4;
    if((u16)entity->velY.part16.fraction<0x800)
        entity->velY.part16.fraction+=32;
    entity->velX.part16.fraction+=entity->velY.part16.fraction;
    if((u16)entity->ext.whiteDragon.swayPhase>0x8000)
    {
        if((u16)entity->ext.whiteDragon.swayPhase<0xFEB0)
            entity->ext.whiteDragon.swayPhase+=320;
    }
    else if((u16)entity->ext.whiteDragon.swayPhase<0x7EB0)
        entity->ext.whiteDragon.swayPhase+=320;
    for(i=99;i>0;i--) history[i]=history[i-1];
    history[0]=entity->velX.part16.fraction;
    if(entity->flipX)
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000) angle+=0x10000;
        if(angle<0x10000)
        {
            angle+=64;
            if(angle>0x10000) angle=0x10000;
        }
        else
        {
            angle-=64;
            if(angle<0x10000) angle=0x10000;
        }
        entity->velX.part16.integer=angle;
    }
    else
    {
        angle=(u16)entity->velX.part16.integer;
        if(angle<0x8000)
        {
            angle+=64;
            if(angle>0x8000) angle=0x8000;
        }
        else
        {
            angle-=64;
            if(angle<0x8000) angle=0x8000;
        }
        entity->velX.part16.integer=angle;
    }
    if(!(entity->timer&3))
        WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
    if(!(entity->timer&15)) PlaySong(0x6E);
    if((s8)--entity->timer<=0) entity->deleteRequested=1;
}

/* Draw the head from two oscillators and the shared body angle. */
#define FIXED_INTEGER(x) ((x)<0 ? -(-(x)>>16) : (x)>>16)
/**
 * @brief Draw function of the head: place it on the neck path from its anchor
 * (unk_4FC) with the sway and wave angles, then draw the sprite.
 */
void WhiteDragon_Draw(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 wave, sway, x, y;
    entity->posX.part16.integer=entity->ext.whiteDragon.anchorX-camera->xPos.part16.integer+entity->ext.whiteDragon.offsetX;
    entity->posY.part16.integer=entity->ext.whiteDragon.anchorY-camera->yPos.part16.integer+entity->ext.whiteDragon.offsetY;
    wave=(Sin((u16)entity->velX.part16.fraction)*entity->ext.whiteDragon.sway)/4096;
    sway=Sin((u16)entity->ext.whiteDragon.swayPhase)/16;
    x=FIXED_INTEGER(Sin(entity->velX.part16.integer+wave+sway+0x4000)*entity->velY.part16.integer);
    y=FIXED_INTEGER(Sin(entity->velX.part16.integer+wave+sway)*entity->velY.part16.integer);
    entity->posX.part16.integer+=x;
    entity->posY.part16.integer+=y;
    Sprite_DrawEntity(entity);
}

/* Body segments sample older angles from the head's motion history. */
#define SEGMENT(e) (*(s8 *)&e->ext.whiteDragonSegment.index)
/**
 * @brief Draw function of a neck segment: place it along the owner's (type 11)
 * neck path, 8 px apart (6 while dying), then draw it.
 */
void WhiteDragonSegment_Draw(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=ENTITY_OWNER(entity);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    if(owner && owner->updateFunc && owner->enemyId==11)
    {
        s16 *history=(s16 *)owner->ext.whiteDragon.history;
        u8 state=owner->state;
        s32 spacing=8;
        s32 wave,sway,x,y;
        if(state==2) spacing=6;
        entity->posX.part16.integer=owner->ext.whiteDragon.anchorX-camera->xPos.part16.integer+owner->ext.whiteDragon.offsetX;
        entity->posY.part16.integer=owner->ext.whiteDragon.anchorY-camera->yPos.part16.integer+owner->ext.whiteDragon.offsetY;
        wave=(Sin(history[(SEGMENT(entity)+1)*spacing])*owner->ext.whiteDragon.sway)/4096;
        sway=Sin((u16)owner->ext.whiteDragon.swayPhase)/16;
        x=FIXED_INTEGER(Sin(owner->velX.part16.integer+wave+sway+0x4000)*owner->velY.part16.integer*(9-SEGMENT(entity))/10);
        y=FIXED_INTEGER(Sin(owner->velX.part16.integer+wave+sway)*owner->velY.part16.integer*(9-SEGMENT(entity))/10);
        entity->posX.part16.integer+=x;
        entity->posY.part16.integer+=y;
        Sprite_DrawEntity(entity);
    }
    else EntityDelete(entity);
}

/* Apply damage and choose full destruction or a short reaction to selected attacks. */
void WhiteDragon_OnHit(struct EwramData_EntityData *entity,struct EwramData_EntityData *attacker,u16 *info)
{
    s32 power=Player_CalculateAttackPower(info[2]);
    struct EntityHit *attack=&attacker->hit;
    s32 damage=Hit_ResolveDamage(entity,attacker,info,power);
    s32 zero;
    entity->hitFlashTimer=8;
    {
        u16 hp=entity->hp;
        damage=(u32)damage<<16;
        damage>>=16;
        hp-=damage;
        zero=0;
        entity->hp=hp;
    }
    Entity_ShowDamage(entity,damage,0);
    if((s16)entity->hp<=0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette=entity->basePalette;
        entity->timer=120;
        entity->state=2;
        entity->subState=zero;
        entity->animFrame=1;
    }
    else if(entity->state==0 && (u8)(attack->type-3)<=1)
    {
        entity->state=3;
        entity->animFrame=1;
        entity->timer=zero;
    }
}

/* Linked body segments follow the owner's palette and stagger their destruction.
 * The r6 visibility pointer preserves agbcc's field-pointer allocation.
 * This constraint does not establish original source spelling. */
#define STATE_WORD(e) (*(u16 *)&(e)->state)
struct SegmentFlags { u8 slot:5; u8 hidden:1; u8 flipX:1; u8 flipY:1; };
/**
 * @brief Neck segment update: copy the owner's (type 11) visibility and palette,
 * throw sparks while it dies; delete when the owner is gone.
 */
void WhiteDragonSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (owner && owner->updateFunc && owner->enemyId==11)
    {
        struct SegmentFlags *visibility;
        u8 hidden = owner->hidden;
        visibility = (struct SegmentFlags *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
        visibility->hidden = hidden;
        do { entity->palette = owner->palette; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        if (STATE_WORD(owner)==0x102)
        {
            visibility->hidden = 0;
            if (SEGMENT(entity)<=5 && !(owner->timer & 7))
                WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
            if ((owner->timer>>2)==SEGMENT(entity)+1)
            {
                WhiteDragon_SpawnDeathPiece(entity,entity->posX.whole,entity->posY.whole,3);
                EntityDelete(entity);
                return;
            }
        }
        if (STATE_WORD(owner)==2 && SEGMENT(entity)<=5 &&
           owner->timer<(10-SEGMENT(entity))*8 && !(owner->timer & 7))
            WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
        if (!Enemy_IsStunnedNoFlash(owner))
            Anim_Update(entity);
    }
    else
        EntityDelete(entity);
}

/* Spawn a projectile along the head's current direction. */
/**
 * @brief Spawn a breath fireball from the head in its aim direction (sound 0x77
 * at the caller).
 */
void WhiteDragon_SpawnBreath(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *projectile=EntityCreateInRange(0x1B,0x48,WhiteDragonBreath_Update);
    if(projectile)
    {
        s16 *direction;
        s32 horizontal,vertical,speed;
        if(!WhiteDragonBreath_LoadGraphics(owner,projectile)) EntityDelete(projectile);
        projectile->scrollWithBg=1;
        WhiteDragonBreath_EnableHitBoxes(projectile);
        projectile->spriteLayer=owner->spriteLayer-1;
        ENTITY_CREATOR(projectile)=owner;
        projectile->flipX=owner->flipX;
        if(owner->flipX)
            projectile->posX.part16.integer=owner->posX.part16.integer;
        else
            projectile->posX.part16.integer=owner->posX.part16.integer;
        projectile->posY.part16.integer=owner->posY.part16.integer;
        direction=&owner->accelX.part16.integer;
        horizontal=Sin(*direction+0x4000);
        vertical=Sin(*direction);
        speed=0x40000;
        projectile->velX.whole=FixedMultiply(horizontal,speed);
        projectile->velY.whole=FixedMultiply(vertical,speed);
        projectile->spawnIndex=owner->spawnIndex;
        projectile->basePalette=owner->basePalette;
        projectile->attack=owner->attack;
    }
}

/* A projectile changes size, slows down and expires through five animation phases. */
void WhiteDragonBreath_Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    u32 box0,box1,box2;
    u8 *flags;
    u32 random;
    s32 one;
    u32 vertical;
    u32 *box;
    box0=0x0A0AFBFB;
    box1=0x1010F8F8;
    box2=0x0A0AFBFB;
    random=RandomNumberGenerator();
    one=1;
    flags=ENTITY_RENDER_FLAGS_ADDRESS(entity);
    entity->flipX=random & one;
    vertical=RandomNumberGenerator() & one;
    vertical<<=7;
    *flags=(*flags & 0x7F) | vertical;
    switch(entity->state)
    {
        case 0:
            EntityMove(entity);
            if(entity->timer++<=5)
            {
                Hit_CheckWithPlayer(&result,entity);
                break;
            }
            entity->state++;
            entity->timer=0;
            entity->animFrame++;
            entity->spriteLayer++;
            box=&box0;
            goto set_boxes;
        case 1:
            EntityMove(entity);
            if(entity->timer++<=5)
            {
                Hit_CheckWithPlayer(&result,entity);
                break;
            }
            entity->state++;
            entity->timer=0;
            entity->spriteLayer++;
            entity->animFrame++;
            box=&box1;
            goto set_boxes;
        case 2:
            EntityDampVelocityX(entity,6);
            EntityDampVelocityY(entity,6);
            EntityMove(entity);
            if(entity->timer++<=13)
            {
                Hit_CheckWithPlayer(&result,entity);
                break;
            }
            entity->state++;
            entity->timer=0;
            entity->animFrame++;
            box=&box2;
set_boxes:
            HitBox_SetBody(entity,box);
            HitBox_SetAttack(entity,box);
            Hit_CheckWithPlayer(&result,entity);
            break;
        case 3:
            if(entity->timer++>3)
            {
                entity->state++;
                entity->timer=0;
                entity->animFrame++;
            }
            Hit_CheckWithPlayer(&result,entity);
            break;
        case 4:
            if(entity->timer++>3)
                EntityDelete(entity);
            break;
    }
}

/* WhiteDragonBreath_EnableHitBoxes (hit boxes). */

/**
 * @brief 777B0 | Activate box slot 8 with WhiteDragonBreath_DamagePlayerOnContact and set both boxes to
 * the bytes of 0x0808FCFC (x -4, y -4, w 8, h 8 - not a ROM address);
 * clear flags bit 1, set bit 2.
 */
void WhiteDragonBreath_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0808FCFC;

    Hit_Enable(entity, 8, WhiteDragonBreath_DamagePlayerOnContact, 0);
    HitBox_SetBody(entity, &box);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}

/* Short lived sparks emitted by the linked body. */
/**
 * @brief Spawn a short spark at (x, y) on layer, randomly flipped
 * (WhiteDragonSpark_Update, 9 frames).
 */
void WhiteDragon_SpawnSpark(struct EwramData_EntityData *owner,s32 x,s32 y,s32 layer)
{
    s32 graphics;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,WhiteDragonSpark_Update);
    if(effect)
    {
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=RandomNumberGenerator()%2;
        effect->spriteLayer=layer;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->basePalette);
        effect->animFrame=9;
        if(owner->flipX)
            effect->posX.part16.integer+=RandomNumberGenerator()%6;
        else
            effect->posX.part16.integer-=RandomNumberGenerator()%6;
        effect->posY.part16.integer+=RandomNumberGenerator()%6-3;
        effect->scrollWithBg=1;
        effect->drawFunc=Sprite_DrawEntity;
    }
}

/* Create a spark that converges on a camera independent target. */
/**
 * @brief Spawn a charge particle near (x, y) that converges on the mouth over 8
 * frames before the breath (WhiteDragonChargeParticle_Update).
 */
void WhiteDragon_SpawnChargeParticle(struct EwramData_EntityData *owner,s32 x,s32 y)
{
    s32 graphics;
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,WhiteDragonChargeParticle_Update);
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    if(effect)
    {
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=RandomNumberGenerator()%2;
        effect->spriteLayer=owner->spriteLayer+1;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->basePalette);
        effect->animFrame=9;
        if(owner->flipX)
            effect->posX.part16.integer+=(u16)(RandomNumberGenerator()%8+16);
        else
            effect->posX.part16.integer-=(u16)(RandomNumberGenerator()%8+16);
        effect->posY.part16.integer+=RandomNumberGenerator()%32+0xFFF0;
        effect->ext.whiteDragonCharge.targetX=owner->posX.part16.integer+camera->xPos.part16.integer;
        effect->ext.whiteDragonCharge.targetY=owner->posY.part16.integer+camera->yPos.part16.integer;
        effect->scrollWithBg=1;
        effect->drawFunc=Sprite_DrawEntity;
    }
}

/* A short spark converges on a saved world position while the camera moves. */
void WhiteDragonChargeParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 dx,dy,remaining;
    entity->flipX=RandomNumberGenerator()%2;
    entity->flipY=RandomNumberGenerator()%2;
    dx=(((s16)entity->ext.whiteDragonCharge.targetX<<16)-(entity->posX.whole+camera->xPos.whole));
    remaining=8-entity->timer;
    dx/=remaining;
    dy=((entity->ext.whiteDragonCharge.targetY<<16)-(entity->posY.whole+camera->yPos.whole))/remaining;
    entity->posX.whole+=dx;
    entity->posY.whole+=dy;
    if(entity->timer++>8)
        EntityDelete(entity);
}

/* Falling debris released by a body segment. */
/**
 * @brief Spawn a falling death piece (frame) at (x, y) facing like owner
 * (WhiteDragonPiece_Update).
 */
void WhiteDragon_SpawnDeathPiece(struct EwramData_EntityData *owner,s32 x,s32 y,s32 frame)
{
    struct EwramData_EntityData *effect=EntityCreateInRange(0x49,0xCF,WhiteDragonPiece_Update);
    if(effect)
    {
        s32 graphics;
        
        effect->posX.whole=x;
        effect->posY.whole=y;
        effect->flipX=owner->flipX;
        effect->spriteLayer=owner->spriteLayer;
        graphics=ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);
        Sprite_Attach(effect,sQuezlcoatlWhiteDragonSpriteSheet,graphics,owner->basePalette);
        effect->animFrame=frame;
        effect->velX.whole=(RandomNumberGenerator()&0x7FFF)-0x4000;
        effect->velY.whole=0;
        effect->accelY.whole=0x1000;
        effect->timer=44;
        effect->scrollWithBg=1;
        effect->drawFunc=Sprite_DrawEntity;
    }
}

/* White Dragon helpers 0x08077B3C..0x08077CBB (graphics shared with Quezlcoatl). */

/**
 * @brief 77B3C | Give projectile the White Dragon / Quezlcoatl graphics (palette row of
 * entity), frame 5; 0 when no graphics slot is free.
 */
u8 WhiteDragonBreath_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sQuezlcoatlWhiteDragonSpriteSheet, gfx, entity->basePalette);
    projectile->animFrame = 5;
    projectile->drawFunc = Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 77B80 | Player_ApplyContactDamage with (attack, 2, 0, 1) at the entity position.
 */
void WhiteDragonBreath_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 77BB0 | Random flips each frame; delete after 9 frames.
 */
void WhiteDragonSpark_Update(struct EwramData_EntityData *entity)
{
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    if (entity->timer++ > 8)
        EntityDelete(entity);
}

/**
 * @brief 77BFC | Fall (max 0x70000), blink, delete after timer.
 */
void WhiteDragonPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x70000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)entity->timer-- <= 0)
        EntityDelete(entity);
}

/**
 * @brief 77C40 | Load the White Dragon graphics (palette 0 of the set shared with Quezlcoatl) (drawn by WhiteDragon_Draw);
 * 0 when no slot is free.
 */
u8 WhiteDragon_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sQuezlcoatlWhiteDragonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sQuezlcoatlWhiteDragonPalette, 0);
    Sprite_Attach(entity, sQuezlcoatlWhiteDragonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = WhiteDragon_Draw;
    return 1;
}

/**
 * @brief 77C8C | Player_ApplyContactDamage with (attack, 0, 0, 1) at the entity position.
 */
void WhiteDragon_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}
