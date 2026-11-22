/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BgEffect_Bg2InterlacedWaveCopy = BgEffect_Bg2InterlacedWave
 *   BgEffect_Bg2PerspectiveCopy = BgEffect_Bg2Perspective */
#include "code_08002454.h"
#include "code_08009A0.h"
#include "code_08001004.h"
#include "code_08005894.h"
#include "code_08008750.h"
#include "code_080096AC.h"
#include "code_0800B700.h"
#include "code_0800CB00.h"
#include "code_0800F1FC.h"
#include "code_08011DD0.h"
#include "code_08012744.h"
#include "code/code_08014548.h"
#include "code/code_080211F0.h"
#include "code_08032444.h"
#include "code_08032E4C.h"
#include "code_08033CAC.h"
#include "code_08035930.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "gba.h"
#include "macros.h"
#include "agb_sram.h"
#include "syscalls.h"
#include "constants/main.h"
#include "constants/songs.h"
#include "data/data_0E0334.h"
#include "data/data_0E3464.h"
#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "code_08001194.h"
#include "code_08013960.h"

/**
 * @brief 65600 | Same as BgEffect_Bg2InterlacedWave.
 */
void BgEffect_Bg2InterlacedWaveCopy(void)
{
    s32 angle;
    s16 *hBlankBuf;
    s32 y;
    s32 scanline;
    s32 zeroIndex;
    u16 vOfs;

    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    angle = gEwramData->frameCounter;
    angle <<= 0xA;
    vOfs = gDisplayRegisters.bgOfs[2].vOfs;
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, REG_BG2HOFS);

    for (scanline = 0, zeroIndex = 2; scanline < SCREEN_SIZE_Y; scanline++, zeroIndex += 3)
    {
        y = scanline + vOfs;
        if (y > 0x120 || (y & 1) == (gEwramData->frameCounter & 0x10) / 0x10)
            hBlankBuf[zeroIndex] = 0;
        else
            *hBlankBuf = Sin(angle) >> 0xF;

        hBlankBuf++;
        angle -= 0x2000;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief 656BC | Same as BgEffect_Bg2Perspective.
 */
void BgEffect_Bg2PerspectiveCopy(void)
{
    s32 sx;
    s32 dist;
    s32 scanline;
    s32 texY;
    struct BgAffineDstData *hBlankBuf;
    u32 scroll;
    struct BgAffineSrcData src;

    scroll = gEwramData->gameState.bgEffectFrameCounter;
    hBlankBuf = (struct BgAffineDstData *)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    texY = 0;
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 0x10, REG_BG2PA);

    for (scanline = 0; scanline < SCREEN_SIZE_Y; scanline++)
    {
        src.texX = 0x8000 - (0x100 * scroll);
        src.scrX = 0x78;
        src.scrY = 0;
        if (scanline < 0x80)
        {
            dist = (0x80 - scanline) << 5;
            sx = Div(0x80000, dist + 0x400);
            src.sx = sx;
            texY -= sx;
            src.texY = texY - (scroll << 7);
        }
        else
        {
            dist = scanline << 7;
            sx = Div(0x80000, dist - 0x3C00);
            src.sx = sx;
            texY -= sx * 2;
            src.texY = texY + (scroll << 7);
        }
        src.sy = 0x100;
        src.alpha = 0;
        BgAffineSet(&src, hBlankBuf, 1);
        hBlankBuf++;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}
