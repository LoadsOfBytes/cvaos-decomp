/* Per-scanline affine transition driven by the owner's event phase. */
#include "code_08009A0.h"
#include "event_scene30.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "gba.h"
#include "syscalls.h"
extern u8 sEventScene30TransformationPalette[],sEventScene30Palette[];
extern void EventScene30_SpawnFloatingFragment(struct EwramData_EntityData *,s16,s16,s32);
#define PHASE(e) (e->ext.event30.phase)
#define CENTER_X(e) (e->ext.event30.centerX)
#define CENTER_Y(e) (e->ext.event30.centerY)
#define SCALE_X(e) (e->ext.event30.scale)
#define SCALE_Y(e) (e->ext.event30.scaleY)
#define SWEEP(e) (e->ext.event30.sweep)
/**
 * @brief Unreferenced: H-blank BG2 affine zoom sequence in phases that also
 * spawns EventScene30 floating fragments.
 */
void EventScene30_UpdateUnusedZoom(struct EwramData_EntityData *entity)
{
    struct BgAffineDstData *out=(struct BgAffineDstData *)&gEwramData->hBlankEffect.hBlankBuffer[1-gEwramData->hBlankEffect.currentBuffer];
    s32 dx=0;
    s32 dy;
    s32 line;
    struct BgAffineSrcData src;
    HBlankEffectSetup(0,160,16,REG_BG2PA);
    switch(PHASE(entity))
    {
    case 0:
        PHASE(entity)++;
        CENTER_X(entity)=152;
        CENTER_Y(entity)=96;
        SCALE_X(entity)=208;
        SCALE_Y(entity)=208;
        entity->ext.raw.unk_50C.part16.unk_50C=0;
        entity->ext.event30.zoomTexY=128;
        break;
    case 1:
        entity->timer=32;
        PHASE(entity)++;
        SWEEP(entity)=0;
        break;
    case 2:
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            src.sx=208;src.sy=208;src.alpha=0;
            BgAffineSet(&src,out,1);
            out++;
        }
        if(--entity->timer==255)
        {SWEEP(entity)=0;PHASE(entity)++;entity->timer=0;}
        break;
    case 3:
        BgPalette_LoadRange(sEventScene30TransformationPalette,0,1,0);
        if((u8)(entity->timer%24)==0)
        {
            s32 x=(s16)((u16)(CENTER_X(entity)+24)-RandomNumberGenerator()%48-16);
            u32 randY=RandomNumberGenerator(); s32 y=(u16)CENTER_Y(entity); y=(s16)(y-(randY%128)+48);
            EventScene30_SpawnFloatingFragment(entity,x,y,0);
        }
        dx=2-(RandomNumberGenerator()&3);
        dy=2-(RandomNumberGenerator()&3);
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            if(entity->timer&1){src.scrX+=dx;src.scrY+=dy;}
            src.sx=208;src.sy=208;src.alpha=0;
            BgAffineSet(&src,out,1);
            out++;
        }
        if(++entity->timer==129)
        {SWEEP(entity)=0;entity->timer=0;PHASE(entity)++;}
        break;
    case 4:
        for(line=0;line<160;line++)
        {
            if(line<80)
            {
                src.texX=0;src.texY=(s16)entity->ext.event30.zoomTexY<<8;
                src.scrX=0;src.scrY=CENTER_Y(entity)-line;
                dx=0;
                if(line<(u16)SWEEP(entity))dx=(u16)SWEEP(entity)-line;
                src.sx=SCALE_X(entity)-dx;src.sy=SCALE_Y(entity);src.alpha=0;
            }
            else
            {
                src.texX=0xA000;src.texY=(s16)entity->ext.event30.zoomTexY<<8;
                src.scrX=200;src.scrY=CENTER_Y(entity)-line;
                dx=0;
                if(160-line<(u16)SWEEP(entity)){ s32 amount=(u16)SWEEP(entity)-160; dx=amount+line; }
                src.sx=SCALE_X(entity)-dx;src.sy=SCALE_Y(entity);src.alpha=0;
            }
            BgAffineSet(&src,out,1);
            out++;
        }
        SWEEP(entity)++;
        if((entity->timer%16)==0)SCALE_Y(entity)++;
        entity->timer++;
        entity->ext.event30.timer++;
        break;
    case 5:
        BgPalette_LoadRange(sEventScene30TransformationPalette,8,1,0);
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity);src.scrY=CENTER_Y(entity)-line;
            src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);
            src.alpha=(u8)entity->ext.event30.angle<<8;
            BgAffineSet(&src,out,1);
            out++;
        }
        if(entity->timer++>128)
        {SWEEP(entity)=0;entity->timer=0;PHASE(entity)=8;}
        break;
    case 8:
        BgPalette_LoadRange(sEventScene30TransformationPalette,9,1,0);
        ObjPalette_LoadRange(sEventScene30Palette,1,1,entity->basePalette);
        for(line=0;line<160;line++)
        {
            src.texX=0x8000;src.texY=0x8000;
            src.scrX=CENTER_X(entity)+dx;src.scrY=CENTER_Y(entity)-line;
            src.sx=SCALE_X(entity);src.sy=SCALE_Y(entity);
            src.alpha=(u8)entity->ext.event30.angle<<8;
            BgAffineSet(&src,out,1);
            out++;
        }
        if((u16)SCALE_Y(entity)<=65534)SCALE_Y(entity)+=64;
        break;
    }
    gEwramData->hBlankEffect.requestStart=1;
}
