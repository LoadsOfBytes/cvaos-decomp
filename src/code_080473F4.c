#include "code/code_08039340.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_08011DD0.h"
#include "code_08013960.h"

/**
 * @brief 473F4 | Enter the menu: unless stateFlags bit 0 is set, reset the
 * game screen, set up blending / BG offsets and remember the equipped
 * blue and yellow souls in sharedScratchBuffer[0x14..0x15].
 */
void PauseMenu_Enter(struct GameState *gameState)
{
    if (gEwramData->gameState.stateFlags & 1)
        return;
    AreaName_Dismiss(gameState);
    Display_SaveState();
    ObjAffine_SaveState();
    ObjGfx_SaveState();
    ItemIconCache_Save();
    Palette_SaveState();
    gUnk_03002CB0.dispCnt &= ~0x2000;
    gEwramData->bgControlMinWriteLevel = gEwramData->unk_A074_4 = 3;
    gDisplayRegisters.bldCnt = 0xFF;
    gEwramData->startPauseEnabled = 0;
    gEwramData->paused = 1;
    gameState->subMode = 6;
    gameState->subModeStep = 0;
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 0;
    gEwramData->hBlankEffect.requestStop = 1;
    Sound_StopTrackedWithVolume();
    BgMap_SaveSelection(1);
    gEwramData->sharedScratchBuffer[0x14] = gEwramData->playerStatus.equippedBlueSoul;
    gEwramData->sharedScratchBuffer[0x15] = gEwramData->playerStatus.equippedYellowSoul;
    Obj_SetBgLayer(2, 0);
    Obj_SetBgLayer(1, 1);
    Bg_SyncPriorities();
    gEwramData->screenShakeOffsetX = 0;
    gEwramData->screenShakeOffsetY = 0;
    gDisplayRegisters.bldY = 0x10;
    gEwramData->pauseMenuPage = 0;
}
