#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code_08011DD0.h"
#include "code_08012744.h"
#include "code_08013960.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba/dma.h"

extern void TextWindow_Close(void);

/**
 * @brief Update of event scene variant 29 (sEventSceneUpdateFuncs[29]); returns the
 * player input mask. Scene state machine: wait for the fade, clear BG
 * VRAM and show BG0, count down 0xFF frames, fade to black, then save
 * (slot currentSaveSlot) and switch to game mode 0xA. Always returns 0.
 */
s32 EventScene29_Update(struct EwramData_EntityData *entity)
{
    s32 ret = 0;

    switch (entity->state)
    {
    case 0:
        switch (entity->subState)
        {
        case 0:
            if (entity->stage == 0)
            {
                Music_StartOverride(0x25);
                entity->stage++;
            }
            if (BgPalette_GetPendingFades() == 0)
            {
                entity->subState++;
                entity->stage = ret;
            }
            else
            {
                gUnk_03002CB0.dispCnt &= ~0x100;
            }
            break;
        case 1:
            DMA_FILL_32(3, ret, 0x06004000, 0x2000);
            DMA_FILL_16(3, ret, 0x0600E000, 0x800);
            gUnk_03002CB0.dispCnt |= 0x100;
            entity->timer = 0xFF;
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
            break;
        }
        break;
    case 1:
        if (--entity->timer == 0xFF)
        {
            BgPalette_StartColorFade(0, 0, 0, 0x10000, ret, 0x80);
            ObjPalette_StartColorFade(0, 0, 0, 0x10000, ret, 0x80);
            gUnk_03002CB0.dispCnt &= ~0x100;
            GameFlag_Set(0x2D);
            entity->state++;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 2:
        if (BgPalette_GetPendingFades() == 0)
        {
            gEwramData->gameState.bossEventFlags &= ~1;
            GameFlag_Clear(0x28);
            SaveData_ResaveSlotAfterEnding(gEwramData->gameState.currentSaveSlot, 1);
            entity->state = 0x80;
            TextWindow_Close();
            gEwramData->gameState.subMode = 0xA;
        }
        break;
    }
    gEwramData->gameState.stateFlags |= 0x20;
    Anim_Update(entity);
    return ret;
}
