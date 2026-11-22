/* Event actor: dialogue selection, fade and optional walk to the player. */
#include "code_08009A0.h"
#include "code_08011DD0.h"
#include "code_08012744.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
extern void EventScene24_Setup(struct EwramData_EntityData *);
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void TextWindow_Close(void);
extern void Sound_Play(u16);
extern void Player_StartScriptedAnimation(s32,s32,s32,s32);
/**
 * @brief Update of event scene variant 24 (sEventSceneUpdateFuncs[24]); returns the
 * player input mask.
 */
s32 EventScene24_Update(struct EwramData_EntityData *entity)
{
    s32 zero=0;
    s32 stage=entity->state;
    switch(stage)
    {
    case 0:
        if(entity->subState==0)
        {
            if(entity->stage==0)
            {
                BgPalette_StartColorFade(31,31,31,0,0x10000,128);
                ObjPalette_StartColorFade(31,31,31,0,0x10000,128);
                entity->stage++;
            }
            else if(BgPalette_GetPendingFades()){entity->subState++;entity->stage=zero;}
        }
        else
        {
            EventScene24_Setup(entity);entity->state++;entity->stage=zero;entity->subState=zero;entity->timer=255;
        }
        goto holdPlayer;
    case 1:
        if(--entity->timer==255)
        {
            s32 selected=0;
            if(SoulInventory_GetCompletionTenths()>799)selected=1;
            if(selected){TextWindow_Begin(38);entity->unk_4F4.part8.unk_4F7=stage;}
            else {TextWindow_Begin(39);entity->unk_4F4.part8.unk_4F7=zero;}
            entity->state++;entity->stage=0;entity->subState=0;
        }
        goto holdPlayer;
    case 2:
        if(TextWindow_Update())
        {
            entity->state++;entity->stage=zero;entity->subState=zero;
            BgPalette_StartColorFade(0,0,0,0x10000,zero,255);
            ObjPalette_StartColorFade(0,0,0,0x10000,zero,255);
            gUnk_03002CB0.dispCnt&=0xFEFF;
            SaveData_RecordFullClear();Sound_Play(0x1040);
        }
        else if(entity->subState==0)
        {
holdPlayer:
            Player_StartScriptedAnimation(81,0,0,0);
            break;
        }
        else if(entity->subState==1)
        {
            gEwramData->playerState.player->flipX=1;
            if(entity->animScriptIndex!=0)Anim_StartIndexed(entity,0,0,0);
        }
        else gEwramData->playerState.player->flipX=0;
        break;
    case 3:
        if(!BgPalette_GetPendingFades())
        {
            TextWindow_Close();GameFlag_Set(45);gEwramData->gameState.subMode=10;
        }
        else if(entity->unk_4F4.part8.unk_4F7)
        {
            if((s16)gEwramData->playerState.player->posX.part16.integer-12>(s16)entity->posX.part16.integer)
            {
                if(entity->animScriptIndex!=6)Anim_StartIndexed(entity,6,0,1);
                entity->posX.whole+=0x2000;
            }
            else if(entity->animScriptIndex!=1)Anim_StartIndexed(entity,1,0,0);
        }
        break;
    }
    if(entity->state)Anim_Update(entity);
    gEwramData->gameState.stateFlags|=32;
    return zero;
}
