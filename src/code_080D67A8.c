/* Alternate falling particles use the original sweep and restore object graphics. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "gba/memory.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void ChaosDefeatParticle_Update(struct EwramData_EntityData *);
extern s32 RandomSigned(s32);
extern u8 sCommonObjectGfxTiles[];
static inline s32 SweepWave(struct EwramData_EntityData *entity)
{
    return Sin(entity->timer*0xA00)*24;
}
static inline void RestoreEffectGraphics(void)
{
    if(gEwramData->gameState.stateFlags & 8)
    {
        u8 *graphics=sCommonObjectGfxTiles;
        s32 offset=ObjTiles_GetSlotTile((u8)ObjGfx_Find(graphics-4));
        u32 *second;
        offset<<=5;
        DmaQueue_IndirectCopy(0x200,(u32 *)graphics,(u32 *)((u8 *)VRAM_OBJ+offset));
        graphics+=0x200;
        second=(u32 *)((u8 *)VRAM_OBJ+0x400+offset);
        DmaQueue_IndirectCopy(0x200,(u32 *)graphics,second);
        gEwramData->gameState.stateFlags&=~8;
    }
}
/**
 * @brief Run the Chaos defeat particle sweep with its original vertical
 * offsets and layer-0x19 particles, then clean up fades and temporary
 * tiles. Preserve the shared countdown labels and fixed-point form.
 */
void ChaosDefeatEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram=gEwramData;
    s32 timer;
    switch(entity->state)
    {
        case 0:
            entity->timer=8;
            entity->accelX.whole=0x20000;
            entity->state++;
        case 1:
            timer=entity->timer;
            if(timer!=0)
                goto tick;
            goto advance;
        case 2:
            if(entity->timer!=0)
            {
                if((entity->timer & 3)==0)
                {
                    struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,(u32 *)ChaosDefeatParticle_Update);
                    if(particle)
                    {
                        s32 random,width,centerX;
                        s32 jitterY,positionY,sweepY;
                        particle->scrollWithBg=1;
                        random=RandomSigned(4);
                        width=entity->accelX.whole;
                        centerX=entity->posX.whole;
                        
                        particle->posX.whole=centerX+(SweepWave(entity)<0 ? random-((-SweepWave(entity))>>16) : (SweepWave(entity)>>16)+random)*width;
                        jitterY=(u32)RandomSigned(16)<<16;
                        jitterY+=0x180000;
                        positionY=entity->posY.whole+jitterY;
                        sweepY=entity->timer<<14;
                        sweepY-=0x80000;
                        particle->posY.whole=positionY+sweepY;
                        particle->velY.whole=0x4000;
                        particle->accelY.whole-=0x2000;
                        if((entity->timer & 7)==0)
                            PlaySong(0x77);
                    }
                }
                entity->accelX.whole-=0x600;
                timer=entity->timer;
                goto tick;
            }
            else
            {
                if(!(ewram->gameState.stateFlags & 0x10))
                {
                    BgPalette_StopColorFade();
                    ObjPalette_StopColorFade();
                }
advance:
                entity->timer=64;
                entity->state++;
            }
            break;
        case 3:
            timer=entity->timer;
            if(timer==0) goto finished;
tick:
            entity->timer=timer-1;
            break;
finished:
            RestoreEffectGraphics();
            EntityDelete(entity);
            return;
    }
    if(!entity->roomActive)
        RestoreEffectGraphics();
}
