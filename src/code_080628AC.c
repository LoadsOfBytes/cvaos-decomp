/* Set up, play and fade the background transformation scene. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
extern u8 sEventScene30TransformationGfx[],sUnk_08119898[],sEventScene30TransformationPalette[];
extern struct BgMapMetadata sEventScene30BgMap,sEventScene30TransformationBg2Map;
extern void EventScene30_UpdateTransformation(struct EwramData_EntityData *);
extern void EventScene30_UploadFrame(void);
/**
 * @brief Event scene 30 sequence: set up the BG layers and graphics, run
 * EventScene30_UpdateTransformation with brightness fade-in, wait, fade
 * to white and clear the maps; return 1 when done.
 */
s32 EventScene30_PlayTransformation(struct EwramData_EntityData *entity)
{
    s32 done=0;
    switch(entity->subState)
    {
    case 0:
        if(entity->stage==0){entity->timer=4;entity->stage++;}
        gUnk_03002CB0.dispCnt=0;
        gDisplayRegisters.bgOfs[0].hOfs=0;gDisplayRegisters.bgOfs[0].vOfs=0;
        gDisplayRegisters.bgOfs[1].hOfs=0;gDisplayRegisters.bgOfs[1].vOfs=0;
        gDisplayRegisters.bgOfs[2].hOfs=0;gDisplayRegisters.bgOfs[2].vOfs=0;
        gDisplayRegisters.bldCnt=255;gDisplayRegisters.bldY=16;{u16 alpha=0x1000;gDisplayRegisters.bldAlpha=alpha;}
        gEwramData->gameModeCounter=0;gEwramData->gameModeTimer=0;
        if(--entity->timer==255){BgPalette_StopColorFade();ObjPalette_StopColorFade();entity->subState=1;}
        break;
    case 1:
        gUnk_03002CB0.dispCnt=0x1E01;
        gDisplayRegisters.bgCnt[1]=0x1D09;gDisplayRegisters.bgCnt[2]=0x5E80;
        Obj_SetBgLayer(22,1);Obj_SetBgLayer(6,2);Bg_SyncPriorities();
        Gfx_CopyResource(sEventScene30TransformationGfx,(void *)0x06000000,0);
        Gfx_CopyResource(sUnk_08119898,(void *)0x06008000,0);
        BgPalette_LoadRange(sEventScene30TransformationPalette,0,16,0);
        BgMap_Setup(1,&sEventScene30BgMap,0,48);BgMap_Setup(2,&sEventScene30TransformationBg2Map,0,0);
        entity->subState=2;
        break;
    case 2:
        EventScene30_UpdateTransformation(entity);EventScene30_UploadFrame();
        gDisplayRegisters.bldY=16-(gEwramData->gameModeTimer>>1);
        if(gDisplayRegisters.bldY)
        {if(entity->timer%8==0)gEwramData->gameModeTimer++;}
        else {gDisplayRegisters.bldCnt=0x3F40;gEwramData->gameModeTimer=0;entity->subState=3;}
        entity->timer++;
        break;
    case 3:
        EventScene30_UpdateTransformation(entity);EventScene30_UploadFrame();
        if(entity->unk_4F8.part8.unk_4F9==10)
        {if(gEwramData->gameModeTimer++>50){gEwramData->gameModeTimer=0;entity->subState=4;}}
        break;
    case 4:
        EventScene30_UpdateTransformation(entity);EventScene30_UploadFrame();
        if(gEwramData->gameModeTimer++>128)
        {
            BgPalette_StopColorFade();ObjPalette_StopColorFade();
            BgPalette_StartColorFade(31,31,31,0x10000,0,64);ObjPalette_StartColorFade(31,31,31,0x10000,0,64);
            gEwramData->gameModeTimer=0;entity->subState=5;
        }
        break;
    case 5:
        EventScene30_UpdateTransformation(entity);EventScene30_UploadFrame();
        if(++gEwramData->gameModeTimer>64)
        {
            DMA_FILL_32(3,0,(void *)0x0600E800,0x800);
            DMA_FILL_32(3,0,(void *)0x0600F000,0x800);
            DMA_FILL_32(3,0,(void *)0x0600F800,0x800);
            entity->subState=6;
        }
        break;
    case 6:
        done=1;
        break;
    }
    gEwramData->gameModeCounter++;
    return done;
}
