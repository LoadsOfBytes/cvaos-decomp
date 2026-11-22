/* Prepare a text event window and reset its execution state. */
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
/**
 * @brief Prepare the text window graphics state, select its localized command stream and reset the window execution state.
 */
void TextWindow_Begin(s32 textId)
{
    struct EwramData *ram = gEwramData;
    gUnk_03002CB0.dispCnt &= 0xFEFF;
    ram->gameState.stateFlags |= 0x21;
    Text_SetTileBuffer((void *)0x06004000);
    Text_SetBackgroundColor(0);
    Text12_SetRegion(0, 0, 31, 4);
    Text12_ClearRegion();
    gDisplayRegisters.win0H = 0x08E8;
    gDisplayRegisters.win0V = 0xC0C1;
    gDisplayRegisters.winIn_L = 0x3F;
    gDisplayRegisters.winOut_L = 0x1F;
    ram->gameState.conversationText = Text_GetLocalizedEntry(textId);
    ram->gameState.insertedTextAddress = 0;
    ram->gameState.textWindowState = 0;
    ram->gameState.insertedTextOffset = 0;
    ram->gameState.conversationTextOffset = 0;
    ram->gameState.textWindowPause = 0;
    ram->gameState.textCharacterDelay = 1;
}
