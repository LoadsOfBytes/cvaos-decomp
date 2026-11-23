/* EventScene30_UpdateTransformation: multi-phase scene transition with per-scanline affine distortion.
   Built without -f2003-patch; the callback table of emitCenter is this file's
   .rodata and is linked at 0x08118B34. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "gba.h"
#include "syscalls.h"
#include <string.h>
extern u8 sEventScene30TransformationPalette[];
extern void EventScene30EmitterA_Update(struct EwramData_EntityData *);
extern void EventScene30EmitterB_Update(struct EwramData_EntityData *);
extern void EventScene30EmitterC_Update(struct EwramData_EntityData *);
extern void EventScene30EmitterD_Update(struct EwramData_EntityData *);
extern void EventScene30EmitterE_Update(struct EwramData_EntityData *);

extern void EventScene30_CreateCenterEffect(struct EwramData_EntityData *);
extern void EventScene30_CreateHalo(struct EwramData_EntityData *);
extern void EventScene30_StepPaletteTransition(struct EwramData_EntityData *);
extern void EventScene30PieceEmitter_Update(struct EwramData_EntityData *);
extern void EventScene30_SpawnOrbitRing(struct EwramData_EntityData *,s16,s16);
extern void EventScene30_SpawnDriftFragment(s16,s16,s32,s32);
extern void PlaySong(u16);
#define PHASE(e) (e->ext.event30.phase)
#define INIT(e) (e->ext.event30.phaseInit)
#define ANGLE(e) (e->ext.event30.angle)
#define CENTER_X(e) (e->ext.event30.centerX)
#define CENTER_Y(e) (e->ext.event30.centerY)
#define SCALE_X(e) (e->ext.event30.scale)
#define SCALE_Y(e) (e->ext.event30.scaleY)
#define SWEEP(e) (e->ext.event30.sweep)
#define TIMER(e) (e->ext.event30.timer)
#define JITTER_X(e) ((e)->ext.raw.unk_508.part8.unk_50A)
#define JITTER_Y(e) (EVENT30_JITTER_Y((e)))
static inline void emitCenter(s16 x,s16 y,u32 index)
{
    u32 *callbacks[5]={(u32 *)EventScene30EmitterA_Update,(u32 *)EventScene30EmitterB_Update,(u32 *)EventScene30EmitterC_Update,(u32 *)EventScene30EmitterD_Update,(u32 *)EventScene30EmitterE_Update};
    struct EwramData_EntityData *child=EntityCreateInRange(0x1B,0x48,callbacks[index]);
    if(child){child->posX.part16.integer=x;child->posY.part16.integer=y;}
}
#define EMIT_CENTER(index) emitCenter(CENTER_X(entity),CENTER_Y(entity),index)
#define FLASH(from,to,duration) do { \
    BgPalette_StartColorFade(31,31,31,from,to,duration); \
    ObjPalette_StartColorFade(31,31,31,from,to,duration); \
} while(0)
#define DRAW_ROWS(xoffset) do { \
    for(line=0;line<160;line++) { \
        src.texX=0x8000;src.texY=0x8000; \
        src.scrX=CENTER_X(entity)+(xoffset);src.scrY=CENTER_Y(entity)-line; \
        src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);src.alpha=(u8)ANGLE(entity)<<8; \
        BgAffineSet(&src,out,1);out++; \
    } \
} while(0)
void EventScene30_UpdateTransformation(struct EwramData_EntityData *entity)
{
    struct BgAffineDstData *out=(struct BgAffineDstData *)&gEwramData->hBlankEffect.hBlankBuffer[1-gEwramData->hBlankEffect.currentBuffer];
    s32 displacement=0;
    s32 dx=0,dy=0;
    s32 line;
    struct BgAffineSrcData src;
    HBlankEffectSetup(0,160,16,REG_BG2PA);
    switch(PHASE(entity))
    {
    case 0:
        if(INIT(entity)==0)
        {
            CENTER_X(entity)=184;CENTER_Y(entity)=112;
            SCALE_X(entity)=176;SCALE_Y(entity)=176;ANGLE(entity)=0;
            entity->posX.part16.integer=CENTER_X(entity);
            entity->posY.part16.integer=CENTER_Y(entity);
            ObjPalette_LoadRange(sEventScene30TransformationPalette,0,1,14);
            entity->unk_510.part8.basePalette=14;
            TIMER(entity)=16;INIT(entity)++;
        }
        DRAW_ROWS(displacement);
        if(--TIMER(entity)==-1)
        {PlaySong(358);TIMER(entity)=0;PHASE(entity)+=2;INIT(entity)=0;}
        break;
    case 2:
    {
        if(INIT(entity)==0)
        {
            TIMER(entity)=670;INIT(entity)++;
            EventScene30_CreateCenterEffect(entity);EventScene30_CreateHalo(entity);entity->timer=1;
        }
        if(TIMER(entity)>255)
        {
            JITTER_X(entity)=1-(RandomNumberGenerator()%2);
            JITTER_Y(entity)=1-(RandomNumberGenerator()%2);
        }
        else
        {
            JITTER_X(entity)=2-(RandomNumberGenerator()%4);
            JITTER_Y(entity)=2-(RandomNumberGenerator()%4);
        }
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            if(TIMER(entity)&1)
            {src.scrX+=(s8)JITTER_X(entity);src.scrY+=(s8)JITTER_Y(entity);}
            else {JITTER_X(entity)=0;JITTER_Y(entity)=0;}
            src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);src.alpha=(u8)ANGLE(entity)<<8;
            BgAffineSet(&src,out,1);out++;
        }
        if(TIMER(entity)==510){EMIT_CENTER(2);}
        if(TIMER(entity)==287){EMIT_CENTER(1);EMIT_CENTER(2);entity->timer=1;}
        if(TIMER(entity)==255){FLASH(0,0x10000,16);}
        else if(TIMER(entity)==239){FLASH(0,0x10000,32);}
        if((u16)(TIMER(entity)-161)<=93)
        {
            s32 next=(u8)ANGLE(entity)-1;
            if((u8)next>247)
            {
                if(TIMER(entity)%entity->timer==0){ANGLE(entity)=next;entity->timer++;}
            }
            else if(TIMER(entity)%16==0)ANGLE(entity)=next;
        }
        if(TIMER(entity)==192){EMIT_CENTER(3);entity->timer=1;}
        if(TIMER(entity)==160){FLASH(0,0x10000,16);}
        else if(TIMER(entity)==152){FLASH(0,0x10000,32);}
        if(TIMER(entity)<160)
        {
            if(TIMER(entity)>156)ANGLE(entity)++;
            else if(TIMER(entity)%32==0){ANGLE(entity)++;entity->timer++;}
        }
        if(TIMER(entity)%16==0 && (u16)SCALE_X(entity)!=208){SCALE_X(entity)++;SCALE_Y(entity)++;}
        if((s16)(TIMER(entity)%25)==0 && CENTER_X(entity)>120)CENTER_X(entity)--;
        if((s16)(TIMER(entity)%50)==0 && CENTER_Y(entity)>80)CENTER_Y(entity)--;
        if(TIMER(entity)==64){EMIT_CENTER(0);}
        if(TIMER(entity)==32){FLASH(0,0x10000,32);}
        if(TIMER(entity)==48){FLASH(0,0x10000,32);}
        if(--TIMER(entity)==-1)
        {TIMER(entity)=0;entity->timer=0;PHASE(entity)++;INIT(entity)=0;}
        break;
    }
    case 3:
        if(INIT(entity)==0)
        {
            entity->timer=0;entity->unk_518.part16.unk_51A=0;
            entity->unk_518.part8.unk_518=0;INIT(entity)++;
        }
        DRAW_ROWS(0);
        if(entity->timer&1)
        {
            if((u16)SCALE_X(entity)!=208){SCALE_X(entity)++;SCALE_Y(entity)++;}
            else {SWEEP(entity)=0;PHASE(entity)=5;INIT(entity)=0;}
        }
        entity->timer++;
        break;
    case 4:
    case 5:
    {
        s32 regionLine;
        if(INIT(entity)==0)
        {
            EMIT_CENTER(4);FLASH(0,0x10000,255);EntityDeleteGroup(128);
            ANGLE(entity)=0;entity->timer=0;
            *(u8 *)&entity->unk_51C=5;INIT(entity)++;
        }
        JITTER_X(entity)=0;JITTER_Y(entity)=0;
        if((u8)(ANGLE(entity)-1)>229)
        {
            dx=2-(RandomNumberGenerator()&3);dy=2-(RandomNumberGenerator()&3);
            JITTER_X(entity)=dx;JITTER_Y(entity)=dy;
        }
        for(line=0,regionLine=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;src.scrX=CENTER_X(entity);
            if(line>CENTER_Y(entity)-84)
            {
                displacement=0;regionLine++;
                if(regionLine<80)
                {
                    if(regionLine<(u16)SWEEP(entity))displacement=(u16)SWEEP(entity)-regionLine;
                    displacement*=3;src.scrX+=displacement;
                }
                else
                {
                    if(160-regionLine<(u16)SWEEP(entity))
                    {s32 amount=(u16)SWEEP(entity)-160;displacement=amount+regionLine;}
                    displacement*=3;src.scrX-=displacement;
                }
            }
            src.scrY=CENTER_Y(entity)-line;
            if(TIMER(entity)&1){src.scrX+=dx;src.scrY+=dy;}
            src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);src.alpha=(u8)ANGLE(entity)<<8;
            BgAffineSet(&src,out,1);out++;
        }
        if((s16)(TIMER(entity)%5)==0 && CENTER_X(entity)>120)CENTER_X(entity)--;
        if((s16)(TIMER(entity)%10)==0 && CENTER_Y(entity)>80)CENTER_Y(entity)--;
        if(TIMER(entity)==30){FLASH(0x10000,0,96);}
        if(TIMER(entity)==158)
        {
            s32 slot;
            FLASH(0,0x10000,64);EntityDeleteGroup(128);
            for(slot=223;slot>=0;slot--)
            {
                struct EwramData_EntityData *child=&gEwramData->entityData[slot];
                if(child->group==1){ObjAffine_Free(child->affineIndex);EntityDelete(child);}
            }
            EventScene30_SpawnOrbitRing(entity,CENTER_X(entity),CENTER_Y(entity));
        }
        if(TIMER(entity)<160)
        {
            if((u16)SWEEP(entity)<84 && TIMER(entity)%2==0)SWEEP(entity)++;
            if((s16)(TIMER(entity)%5)==0 && (u8)(ANGLE(entity)-1)>191)ANGLE(entity)--;
            if((s16)(TIMER(entity)%10)==0 && TIMER(entity)<80)
            {
                s32 x,y;
                EventScene30_SpawnDriftFragment((s32)TIMER(entity)/10-((RandomNumberGenerator()&15)-136),38-(RandomNumberGenerator()&15),0,0);
                x=(s16)(128-(RandomNumberGenerator()&31)-TIMER(entity)/10);
                y=(s16)(146-(RandomNumberGenerator()&31));
                EventScene30_SpawnDriftFragment(x,y,1,0);
            }
            if((u16)(TIMER(entity)-72)<=16)
            {
                s32 x=(s16)(178-(RandomNumberGenerator()&15));
                s32 y=(s16)(128-(RandomNumberGenerator()&15));EventScene30_SpawnDriftFragment(x,y,1,0);
            }
            if(TIMER(entity)==96)
            {
                struct EwramData_EntityData *child=EntityCreateInRange(0x1B,0x48,(u32 *)EventScene30PieceEmitter_Update);
                if(child){child->posX.part16.integer=140;child->posY.part16.integer=50;child->timer=4;}
            }
            if((u16)(TIMER(entity)-112)<=8)
            {
                s32 x=(s16)(158-(RandomNumberGenerator()%16));
                s32 y=(s16)(112-(RandomNumberGenerator()%8));EventScene30_SpawnDriftFragment(x,y,1,0);
            }
            if((u16)(TIMER(entity)-128)<=16 && (RandomNumberGenerator()&1)==0)
            {
                s32 x=(s16)(72-(RandomNumberGenerator()&7));
                s32 y=(s16)(78-(RandomNumberGenerator()&7));EventScene30_SpawnDriftFragment(x,y,0,0);
            }
            if((TIMER(entity)&3)==0 && TIMER(entity)>112 && TIMER(entity)<152)
            {
                s32 x,y;
                EventScene30_SpawnDriftFragment((s32)TIMER(entity)/10-((RandomNumberGenerator()&15)-136),78-(RandomNumberGenerator()&15),0,0);
                x=(s16)(120-(RandomNumberGenerator()&15)-TIMER(entity)/10);
                y=(s16)(98-(RandomNumberGenerator()&15));EventScene30_SpawnDriftFragment(x,y,1,0);
            }
        }
        else
        {
            if(TIMER(entity)%2==0)
            {
                s32 next=(u8)ANGLE(entity)-1;
                if((u8)next>191)ANGLE(entity)=next;
                else
                {
                    SWEEP(entity)=0;TIMER(entity)=0;entity->timer=0;PHASE(entity)+=2;entity->attack|=2;
                    break;
                }
            }
            if((u16)SCALE_X(entity)+128<=16383)SCALE_X(entity)+=128;else SCALE_X(entity)=16384;
            if((u16)SCALE_Y(entity)!=0)SCALE_Y(entity)-=4;
        }
        TIMER(entity)++;EventScene30_StepPaletteTransition(entity);
        break;
    }
    case 6:
    case 7:
        DRAW_ROWS(0);
        if((u16)SCALE_X(entity)<=65534)SCALE_X(entity)+=1600;
        if(--entity->timer==255){SWEEP(entity)=0;PHASE(entity)++;entity->timer=0;}
        break;
    case 8:
        DRAW_ROWS(displacement);
        if(entity->timer==0)
        {
            s32 xScale=(u16)SCALE_X(entity),yScale=(u16)SCALE_Y(entity);
            s32 step=384;
            if(xScale+8192<=16384)SCALE_X(entity)=xScale+8192;else SCALE_X(entity)=16384;
            if(yScale+step<=16384)SCALE_Y(entity)+=step;
            else {SCALE_Y(entity)=16384;PHASE(entity)++;entity->timer=16;}
        }
        else entity->timer--;
        break;
    case 9:
        if(--entity->timer==255){entity->timer=0;PHASE(entity)++;}
        break;
    case 10:
        DRAW_ROWS(displacement);
        if(entity->timer++&4)SCALE_Y(entity)=16384;else SCALE_Y(entity)=12288;
        break;
    }
    gEwramData->hBlankEffect.requestStart=1;
}
