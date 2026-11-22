/* Finish the event script, fade the scene and resume normal game state. */
#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code_08011DD0.h"
#include "code_08012744.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void TextWindow_Close(void);
extern void Sound_Play(u16);
extern void Player_StartScriptedAnimation(s32,s32,s32,s32);
/**
 * @brief Update of event scene variant 23 (sEventSceneUpdateFuncs[23]); returns the
 * player input mask.
 */
s32 EventScene23_Update(struct EwramData_EntityData *entity)
{
    s32 zero=0;
    gEwramData->gameState.stateFlags|=1;
    Game_SetMenusEnabled(0);
    switch(entity->state)
    {
    case 0:
        if(entity->subState==0)
        {
            DMA_FILL_32(3,zero,(void *)0x06004000,0x2000);
            DMA_FILL_16(3,zero,(void *)0x0600E000,0x800);
            gEwramData->gameState.stateFlags|=32;
            gEwramData->gameState.stateFlags|=0x10000;
            entity->timer=128;entity->subState++;
        }
        if(--entity->timer==255)
        {TextWindow_Begin(35);entity->state++;entity->stage=zero;entity->subState=zero;}
        break;
    case 1:
        if(TextWindow_Update())
        {
            entity->state++;entity->stage=zero;entity->subState=zero;
            BgPalette_StartColorFade(0,0,0,0x10000,zero,255);
            ObjPalette_StartColorFade(0,0,0,0x10000,zero,255);
            gUnk_03002CB0.dispCnt&=0xFEFF;
            GameFlag_Set(44);Sound_Play(0x1040);
        }
        break;
    case 2:
        if(!BgPalette_GetPendingFades()){entity->state=128;entity->stage=zero;entity->subState=zero;}
        break;
    case 128:
        gEwramData->gameState.systemFlags|=1;
        SaveData_SaveSystemFlags();SaveData_ResaveSlotAfterEnding(gEwramData->gameState.currentSaveSlot,0);
        TextWindow_Close();gEwramData->gameState.subMode=10;
        break;
    }
    Player_StartScriptedAnimation(0,0,0,0);
    gEwramData->gameState.stateFlags|=32;
    return zero;
}
