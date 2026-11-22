/* Graham defeat scene: dialogue, graphics cleanup, particles and exit event.
 *: case 3 tails written per branch (CSE reuses the
 * zero register, cross-jump merges them), case 4 re-reads object->subState
 * (GCSE copies), constant 170 in a variable for the player-side compare. */
#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "gba/dma.h"
#include "structs/ewram.h"
#include "structs/main.h"
extern s32 Graham_CreateSceneObject(struct EwramData_EntityData *,s32);
extern struct EwramData_EntityData *Graham_CreateSceneProjectile(s32,s32,s32);
extern void Graham_SpawnSceneProjectile(struct EwramData_EntityData *);
extern void Graham_SpawnBreakupEffect(s32,s32);
extern s32 Player_GetZoneAroundX(s32,s32,s32);
extern u8 sGrahamGfx2[],sGrahamGfx3[],sEndingRoomDefinition[];
#define FLAGS(e) ((e)->unk_4F8.part16.unk_4FA)
/**
 * @brief Root defeat state (0xF0): coordinate dialogue objects, palette/
 * graphics cleanup, timed effects and character-dependent exit events.
 * Gate menus and player input through the existing scene progression.
 */
void Graham_UpdateDefeatScene(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *object=&gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    if(gEwramData->gameState.stateFlags & 0x10) return;
    switch(entity->subState)
    {
        case 0:
        {
            s32 inner=entity->stage;
            switch(inner)
            {
                case 0:
                    if(gEwramData->gameState.bossRushMode==0)
                    {
                        u32 *flags=&gEwramData->gameState.stateFlags;
                        *flags|=0x20;
                        DMA_FILL_32(3,inner,(u8 *)VRAM_BASE+0x4000,0x2000);
                        *flags|=0x10000;
                        if(FLAGS(entity)&8)
                        {
                            if(Graham_CreateSceneObject(entity,3))
                                goto start_dialogue;
                        }
                        else if(Graham_CreateSceneObject(entity,7))
                        {
start_dialogue:
                            entity->stage++;
                            goto mark_dialogue;
                        }
                    }
                    entity->subState++;
                    entity->stage=inner;
mark_dialogue:
                    FLAGS(entity)|=0x10;
                    break;
                case 1:
                    if(object && (!object->updateFunc || object->unk_518.part8.enemyId!=32))
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    break;
            }
            break;
        }
        case 1:
            FLAGS(entity)|=0x20;
            entity->timer=140;
            if(gEwramData->playerStatus.currentCharacter==1)
                PlaySong(0x1DE);
            else
                PlaySong(0x1DB);
            entity->subState++;
            entity->stage=0;
            break;
        case 2:
            if(entity->timer==32)
                Graham_SpawnBreakupEffect(entity->posX.whole,entity->posY.whole-0x80000);
            if(--entity->timer==255)
            {
                entity->subState++;
                entity->stage=0;
                FLAGS(entity)|=0x40;
                gEwramData->gameState.stateFlags&=~2;
            }
            break;
        case 3:
            if(entity->stage==0)
            {
                s32 i=0;
                u8 *graphics=sGrahamGfx2;
                for(;i<4;i++)
                {
                    gEwramData->gameState.sharedObjPaletteSources[i]=0;
                    gEwramData->gameState.sharedObjPaletteVariants[i]=0;
                }
                ObjGfx_Unload(ObjGfx_Find(graphics));
                ObjGfx_Unload(ObjGfx_Find(sGrahamGfx3));
                gEwramData->gameState.bossEventFlags|=1;
                PlaySong(0x1010);
                entity->timer=64;
                entity->stage++;
            }
            if(--entity->timer==255)
            {
                FLAGS(entity)|=0x40;
                gEwramData->gameState.stateFlags&=~2;
                if(gEwramData->gameState.bossRushMode==0)
                {
                    if(gEwramData->playerStatus.currentCharacter==1)
                    {
                        entity->subState=6;
                        entity->stage=0;
                    }
                    else if(FLAGS(entity)&8)
                    {
                        if(Graham_CreateSceneObject(entity,4))
                        {
                            struct EwramData_EntityData *projectile=Graham_CreateSceneProjectile((s16)entity->posX.part16.integer,
                                                                     (s16)(entity->posY.part16.integer-16),1);
                            entity->unk_4FC.part16.unk_4FE[0]=projectile->unk_4F4.part8.unk_4F4;
                            gEwramData->gameState.stateFlags&=~0x10000;
                            entity->subState++;
                            entity->stage=0;
                        }
                        else
                        {
                            entity->subState+=2;
                            entity->stage=0;
                        }
                    }
                    else if(Graham_CreateSceneObject(entity,1))
                    {
                        entity->subState++;
                        entity->stage=0;
                        Sound_PlayTracked(0x16C);
                    }
                    else
                    {
                        entity->subState+=2;
                        entity->stage=0;
                    }
                }
                else
                {
                    entity->subState+=2;
                    entity->stage=0;
                }
            }
            break;
        case 4:
            if(object)
            {
                if(FLAGS(entity)&8)
                {
                    struct EwramData_EntityData *projectile=&gEwramData->entityData[(u16)entity->unk_4FC.part16.unk_4FE[0]];
                    if(!projectile->updateFunc && object->subState==3)
                        object->timer=(u32)projectile->updateFunc;
                    if((u8)(entity->timer%3)==0)
                    {
                        if(object->subState<=6 || (object->subState==7 && object->stage<=1))
                        {
                            if(object->subState==7 && object->stage==1)
                                entity->timer=7;
                            else if(object->subState<=4)
                                entity->timer=7;
                            Graham_SpawnSceneProjectile(entity);
                        }
                    }
                    entity->timer++;
                }
                if(object->state==128)
                {
                    u16 flag=FLAGS(entity)&8;
                    if(flag)
                    {
                        entity->subState++;
                        entity->stage=0;
                    }
                    else
                    {
                        entity->subState+=2;
                        entity->stage=flag;
                    }
                }
            }
            break;
        case 5:
            if(entity->stage==0)
            {
                Player_SetSelectedTarget(0);
                gEwramData->gameState.stateFlags&=~2;
                entity->stage++;
            }
            Game_SetMenusEnabled(1);
            break;
        case 6:
        {
            s32 inner;
            s32 jitterX,jitterY;
            SetPlayerInput(1,0);
            inner=entity->stage;
            if(inner==0)
            {
                Sound_StopTracked(0x16C);
                Sound_PlayTracked(0x105);
                entity->timer=inner;
                BgPalette_StartColorFade(31,31,31,0x10000,inner,255);
                ObjPalette_StartColorFade(31,31,31,0x10000,inner,255);
                entity->stage++;
            }
            else if(inner==1)
            {
                if(entity->timer==100)
                    PlaySong(0x1041);
                if(!BgPalette_GetPendingFades())
                    entity->stage++;
            }
            else if(inner==2)
            {
                if(gEwramData->playerStatus.currentCharacter==0)
                    gEwramData->gameState.stateFlags|=0x2000;
                else
                    gEwramData->gameState.stateFlags|=0x8000;
                gEwramData->startPauseEnabled=0;
                gEwramData->gameState.stateFlags|=0x800000;
                RoomJump_SetDestination(sEndingRoomDefinition,0,0,0,0);
                RoomJump_Begin(&gEwramData->gameState);
                Player_SetSelectedTarget(0);
                gEwramData->gameState.stateFlags&=~2;
                gDisplayRegisters.bldCnt=0;
                entity->stage++;
            }
            jitterX=(s8)(RandomNumberGenerator()%3);
            jitterY=(s8)(RandomNumberGenerator()%3);
            ScreenShake_RequestOffset(2,jitterX,jitterY);
            break;
        }
    }
    if(gEwramData->gameState.bossRushMode==0)
    {
        if(!(FLAGS(entity)&8) && entity->subState>3)
        {
            u32 timer=entity->timer;
            if((u8)(timer%3)==0)
            {
                entity->timer=7;
                Graham_SpawnSceneProjectile(entity);
                entity->timer=timer;
            }
            entity->timer++;
        }
        if((u8)(entity->subState-2)<=1 && gEwramData->gameState.bossRushMode==0)
        {
            s32 input=0;
            s32 x=170;
            if((s8)Player_GetZoneAroundX(x,60,30)==1)
            {
                s32 playerX=(s16)gEwramData->playerState.player->posX.part16.integer;
                input=x<playerX ? 32 : 16;
            }
            if(input==0)
                gEwramData->playerState.player->flipX=1;
            SetPlayerInput(1,input);
        }
    }
    if(entity->subState!=5)
        Game_SetMenusEnabled(0);
    else
        Game_SetMenusEnabled(1);
}
