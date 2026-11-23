/* Seven scene stages: appear, spiral particles, fall, trigger and wait. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_0800B700.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/main.h"

#include "structs/entity_render_flags.h"

extern s32 RandomSigned(s32);
extern s32 Hit_CheckWithPlayer(void *,struct EwramData_EntityData *);
extern void BossRecoveryPickupParticle_Update(struct EwramData_EntityData *);
#define PHASE(e) (e->ext.bossPickup.phase)
#define LANDED(e) (e->ext.bossPickup.landed)
#define SCALE(e) (*(s32 *)&e->ext.bossPickup.scale)
#define EVENT(e) ((e)->spawnParam0)
/**
 * @brief Reveal and grow the delayed pickup, emit particles, fall and
 * wait for player contact. Restore HP/MP and optional Julius stat
 * growth, wait for displayed values, then free the affine slot.
 */
void BossRecoveryPickup_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera=&gEwramData->bgInfo[1];
    s32 bounds[4];
    switch(entity->state)
    {
        case 0:
            if(entity->timer)
            {
                entity->timer--;
                break;
            }
            entity->state=1;
        case 1:
            Sound_PlayTracked(0x14C);
            entity->hidden=0;
            entity->state=2;
        case 2:
            entity->timer++;
            if(!(entity->timer & 1) && SCALE(entity)<=0x7FFF)
            {
                u16 angle=RandomNumberGenerator();
                struct EwramData_EntityData *particle=EntityCreateInRange(0x49,0xCF,(u32 *)BossRecoveryPickupParticle_Update);
                if(particle)
                {
                    particle->accelX.whole=entity->posX.whole+camera->xPos.whole;
                    particle->accelY.whole=entity->posY.whole+camera->yPos.whole;
                    particle->flipX=RandomNumberGenerator() & 1;
                    particle->ext.bossPickupParticle.angle=angle;
                    particle->ext.bossPickupParticle.radius=RandomSigned(0x80000)+0x380000;
                }
            }
            if(SCALE(entity)==0xF000)
            {
                Sound_StopTracked(0x14C);
                PlaySong(0x14B);
            }
            if(SCALE(entity)<=0xFFFF)
                SCALE(entity)+=0x200;
            else
            {
                entity->hit.flags&=~2;
                entity->state=3;
            }
            break;
        case 3:
            entity->timer++;
            if(!(entity->timer & 1))
            {
                s32 x=entity->posX.whole+(u32)RandomSigned(16)*0x10000;
                s32 y=entity->posY.whole+(u32)RandomSigned(16)*0x10000;
                s32 angle=RandomSigned(0x1000);
                struct EwramData_EntityData *particle=Effect_CreateFromTable(x,y,angle,0,4,9);
                if(particle)
                {
                    particle->velY.whole=RandomSigned(0x4000)-0x10000;
                    particle->accelY.whole=-particle->velY.whole/32;
                    particle->spriteLayer=10;
                }
            }
            if(Sin((s16)PHASE(entity))<0)
                SCALE(entity)=-Sin((s16)PHASE(entity))/3+0x10000;
            else
                SCALE(entity)=Sin((s16)PHASE(entity))/3+0x10000;
            EntityAccelerateAndMoveY(entity,0x80000);
            if(!LANDED(entity))
            {
                s32 x=(s16)entity->posX.part16.integer;
                struct BgLayerState *bg=&gEwramData->bgInfo[1];
                s32 y;
                x+=(s16)bg->xPos.part16.integer;
                y=(s16)entity->posY.part16.integer+(s16)bg->yPos.part16.integer;
                if(Collision_GetPushUp(x,y+57) && (gUnk_03002CB0.collisionProbeFlags & 2))
                {
                    entity->accelY.whole=0;
                    LANDED(entity)=1;
                }
            }
            else
            {
                EntityDampVelocityY(entity,6);
                if(entity->velY.whole<=0xFFF)
                    entity->velY.whole=0;
                if(!entity->velY.whole)
                {
                    if(!(PHASE(entity) & 0x7FFF))
                        PlaySong(0x14A);
                    PHASE(entity)+=0x200;
                    entity->posY.whole+=Sin((s16)PHASE(entity)/4)/4;
                }
            }
            { s32 collision=Hit_CheckWithPlayer(bounds,entity);
            collision=(u32)collision<<16;
            collision=(u32)collision>>16;
            collision=(u8)(collision & 1); if(collision)
            {
                entity->state=4;
                PlaySong(0xF8);
                entity->hidden=1;
            }
            }
            break;
        case 4:
        {
            s32 zero;
            if(EVENT(entity))
                Player_ApplyJuliusStatGrowth();
            Player_RestoreHPAndMP();
            zero=0;
            entity->stage=64;
            entity->subState=1;
            Game_SetMenusEnabled(0);
            gEwramData->paused=1;
            entity->timer=zero;
            entity->state=5;
            if(EVENT(entity))
            {
                gEwramData->gameState.bossEventFlags|=1<<EVENT(entity);
                gEwramData->gameState.stateFlags&=~2;
            }
            break;
        }
        case 5:
            entity->subState=0;
            if(gEwramData->gameState.hudHP<gEwramData->playerStatus.currentHP ||
               gEwramData->gameState.hudMP<gEwramData->playerStatus.currentMP)
            {
                if(!(entity->timer & 7))
                    PlaySong(0x140);
                entity->timer++;
            }
            else
                entity->state=6;
            break;
        case 6:
            if(ENTITY_RENDER_FLAGS16(entity) & 0x101F)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
    }
    ObjAffine_Set(0,SCALE(entity),SCALE(entity),entity->flipX,0,entity->affineIndex);
}
