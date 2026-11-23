/* WarpRoom_Update. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_080109F4.h"
#include "code_08011DD0.h"
#include "code_08013960.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "gba/dma.h"
#include "structs/main.h"
#include "structs/ewram.h"

extern void Player_StartScriptedAnimation(s32, s32, s32, s32);

/**
 * @brief Warp room (object 0x1F): on Up at the warp spot open the warp menu;
 * a different destination starts the warp sequence. With
 * stateFlags bit 0x40 start in the arrival state.
 */
void WarpRoom_Update(struct EwramData_EntityData *e)
{
    u16 input = gEwramData->inputData.newInput;
    struct GameState *gameState = &gEwramData->gameState;

    switch (e->state)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 0x40)
        {
            gUnk_03002CB0.dispCnt |= 0x2000;
            e->ext.warpRoom.windowBottom = 0x74;
            gDisplayRegisters.winIn_L = 0x1B;
            gDisplayRegisters.winOut_L = 0x3F;
            gDisplayRegisters.winOut_H = 0x1E;
            gEwramData->playerState.player->posY.part16.integer = 0x60;
            e->state = 5;
            gEwramData->startPauseEnabled = 0;
        }
        else
        {
            gUnk_03002CB0.dispCnt &= 0xDFFF;
            gEwramData->gameState.stateFlags &= ~1;
            e->state = 1;
            gEwramData->startPauseEnabled = 1;
        }
        e->timer = 0;
        break;
    case 1:
        switch (e->subState)
        {
        case 0:
            if (!(input & 0x40))
                break;
            if (!Entity_IsInRect(gEwramData->playerState.player, 0x55, 0x7F, 0x49, 2))
                break;
            Game_SetMenusEnabled(0);
            e->subState++;
            e->timer = 3;
            break;
        case 1:
        {
            s32 r = Player_IsBusy();
            if (r == 0)
            {
                PlaySong(0xF3);
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                RedSoul_CancelAll();
                gEwramData->gameState.stateFlags |= 0x20000;
                gEwramData->startPauseEnabled = 0;
                e->timer = r;
                e->subState = r;
                e->state++;
                WarpMenu_Open(gameState);
            }
            else if (--e->timer == 0xFF)
            {
                Game_SetMenusEnabled(1);
                e->subState--;
            }
            break;
        }
        }
        break;
    case 2:
        if (gEwramData->gameState.selectedWarpIndex == -1)
        {
            e->state = 0;
            e->subState = 0;
            break;
        }
        if (gEwramData->gameState.currentRoomDefinition != gEwramData->gameState.roomJumpDestination)
        {
            PlaySong(0x1010);
            PlaySong(0xF9);
            e->subState = 0;
            e->state++;
            break;
        }
        e->subState = 0;
        e->state = 0;
        break;
    case 3:
        if (e->timer & 3)
            gEwramData->playerState.player->palette = 4;
        else
            gEwramData->playerState.player->palette = 0;
        if (e->timer++ > 0x80)
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 0x40);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 0x40);
            e->timer = 0;
            e->state++;
        }
        break;
    case 4:
        if (e->timer++ > 0x3E)
        {
            gEwramData->gameState.stateFlags |= 0x40;
            GameFlag_Set(0x39);
            Room_ClearForTransition(gameState);
            DMA_FILL_32(3, 0, 0x0600E800, 0x800);
            DMA_FILL_32(3, 0, 0x0600F000, 0x800);
            DMA_FILL_32(3, 0, 0x0600F800, 0x800);
            gEwramData->playerState.player->posX.part16.integer = gEwramData->gameState.roomJumpPlayerX;
            gEwramData->playerState.player->posY.part16.integer = gEwramData->gameState.roomJumpPlayerY;
            gEwramData->playerState.player->velX.whole = 0;
            gEwramData->playerState.player->velY.whole = 0;
            gEwramData->gameState.currentRoomDefinition = gEwramData->gameState.roomJumpDestination;
            Room_LoadAtCamera(gEwramData->gameState.roomJumpCameraX, gEwramData->gameState.roomJumpCameraY);
            Room_StoreEntryPosition(gameState);
        }
        break;
    case 5:
        BgPalette_StopColorFade();
        ObjPalette_StopColorFade();
        BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x40);
        ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x40);
        e->timer = 0;
        e->state++;
        gEwramData->playerState.player->palette = 0;
        break;
    case 6:
        if (e->timer++ > 0x3F)
        {
            BgPalette_StopColorFade();
            ObjPalette_StopColorFade();
            e->state++;
            e->timer = 0;
            gEwramData->gameState.stateFlags &= ~0x40;
            Music_PlayAreaQuiet();
        }
        break;
    case 8:
        gEwramData->startPauseEnabled = 1;
        if ((u8)(e->timer++ % 3) == 0)
        {
            if ((s16)--e->ext.warpRoom.windowBottom < 0x24)
            {
                e->ext.warpRoom.windowBottom = 0x24;
                e->timer = 0;
                e->state = 0;
            }
        }
        break;
    }

    if (e->state >= 3 && e->state <= 5)
    {
        gEwramData->startPauseEnabled = 0;
        Player_StartScriptedAnimation(0x20, 1, 0, 0xFFFFE000);
    }
    if (e->state >= 2 && e->state <= 7)
        SetPlayerInput(1, 0);
    if (e->state > 4)
    {
        s32 h = (s16)e->ext.warpRoom.windowBottom;
        gDisplayRegisters.win0H = 0x5898;
        h |= 0x2400;
        gDisplayRegisters.win0V = h;
    }
}
