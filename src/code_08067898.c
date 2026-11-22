/* Close the event display and release its background/input state. */
#include "code_0800CB00.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
/**
 * @brief Disable the event window display state, restore or clear its background under the existing flags and set execution field textWindowPause to 255.
 */
void TextWindow_Close(void)
{
    gUnk_03002CB0.dispCnt&=0x9FFF;
    gUnk_03002CB0.dispCnt&=0x1FFF;
    gDisplayRegisters.bldY=0;
    if(gEwramData->gameState.stateFlags&0x10000)
    {DMA_FILL_32(3,0,(void *)0x0600E000,0x800);}
    else
    {Hud_Init();gEwramData->gameState.stateFlags&=~32;}
    gEwramData->gameState.textWindowPause=255;
}
