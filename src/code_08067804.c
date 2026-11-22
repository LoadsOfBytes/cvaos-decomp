/* Advance and clamp the event window opening around its vertical center. */
#include "code_0800CB00.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "gba.h"
#include "syscalls.h"
/**
 * @brief Add delta to stored opening amount, clamp a local copy to 0..36 and derive window bounds around Y 124; return one at either local limit.
 */
s32 TextWindow_AdjustOpening(s32 delta)
{
    s32 finished = 0;
    s32 amount;
    gDisplayRegisters.bldY = 8;
    gDisplayRegisters.bldCnt = (gDisplayRegisters.bldCnt & 0xFF3F) | 0xC0;
    gDisplayRegisters.bldCnt |= 0x3F;
    gDisplayRegisters.bldCnt &= 0xFFFE;
    gUnk_03002CB0.dispCnt |= 0x2100;
    gEwramData->gameState.textWindowOpening += delta;
    amount = gEwramData->gameState.textWindowOpening;
    if (amount > 35) { amount = 36; finished = 1; }
    else if (amount <= 0) { amount = 0; finished = 1; }
    {s32 result = Div(amount * 32, 36); gDisplayRegisters.win0V = ((124-result)<<8) | (result+124);}
    return finished;
}


