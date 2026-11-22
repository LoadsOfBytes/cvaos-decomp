/* A camera-relative rain of particles restores shared object graphics at exit. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "gba/memory.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void BossDefeatParticle_Update(struct EwramData_EntityData *);
extern u8 sCommonObjectGfxTiles[];
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
 * @brief Run the Headhunter defeat particle sequence across room-space
 * X 112..384 and Y 128..184, then stop permitted fades and restore
 * the common effect tiles. Retain countdown and ownership behavior.
 */
void HeadhunterDefeatEffect_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
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
            timer=96;
            goto advance;
        case 2:
            if(entity->timer!=0)
            {
                if((entity->timer & 3)==0)
                {
                    struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,(u32 *)BossDefeatParticle_Update);
                    if(particle)
                    {
                        u32 random;
                        s32 x,y;
                        particle->scrollWithBg=1;
                        random=RandomNumberGenerator()%18;
                        x=(random<<4)+112;
                        x-=camera->xPos.part16.integer;
                        particle->posX.part16.integer=x;
                        random=RandomNumberGenerator();
                        y=((random&7)<<3)+128;
                        y-=camera->yPos.part16.integer;
                        particle->posY.part16.integer=y;
                        particle->velY.whole=0x4000;
                        particle->accelY.whole-=0x2000;
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
                timer=64;
advance:
                entity->timer=timer;
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
