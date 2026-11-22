/* A timed sine-shaped sweep emits falling particles during the defeat effect. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BossDefeatParticle_Update(struct EwramData_EntityData *);
extern s32 RandomSigned(s32);
extern void BossDefeat_RestoreEffectTiles(void);
static inline s32 SweepWave(struct EwramData_EntityData *entity)
{
    return Sin(entity->timer*0xA00)*24;
}
/**
 * @brief Unreferenced defeat effect next to GrahamBreakupFragment_Update: a
 * timed sine-shaped sweep that emits falling BossDefeatParticle_Update
 * particles.
 */
void BossDefeat_EmitSweepParticlesUnused(struct EwramData_EntityData *entity)
{
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
                    struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,(u32 *)BossDefeatParticle_Update);
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
                if(!(gEwramData->gameState.stateFlags & 0x10))
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
            BossDefeat_RestoreEffectTiles();
            EntityDelete(entity);
            return;
    }
    if(!entity->roomActive)
        BossDefeat_RestoreEffectTiles();
}
