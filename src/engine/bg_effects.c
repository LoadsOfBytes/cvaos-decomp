#include "bg_effects.h"
#include "core.h"
#include "bg_and_collision.h"
#include "in_game.h"
#include "room_entities.h"
#include "music.h"
#include "gameplay.h"
#include "sound.h"
#include "gba.h"
#include "syscalls.h"

#include "constants/main.h"
#include "constants/songs.h"

#include "data/interrupt_table.h"
#include "data/debug_font.h"

#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "math_anim.h"
#include "menu_common.h"
#include "palette.h"
#include "text12.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "constants/game_flags.h"

extern u16 sEnemyNameTextIds[];

/**
 * @brief BG scroll mode 14: H-blank BG3 horizontal offsets scaled by the play
 * timer, decreasing down the top 80% of the screen and zero below.
 */
void BgEffect_Bg3ScanlineParallax(void)
{
    s16 *hBlankBuf;
    s32 timer;
    s32 scanline;

    timer = gEwramData->gameState.bgEffectFrameCounter;
    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, REG_BG3HOFS);

    for (scanline = 0; scanline < (s32)(SCREEN_SIZE_Y * 0.8f); scanline++)
    {
        *hBlankBuf++ = (timer * (0x40 - scanline / 2)) / 0x10;
    }

    for (scanline = 0; scanline < (s32)(SCREEN_SIZE_Y * 0.2f); scanline++)
    {
        *hBlankBuf++ = 0;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief H-blank sine wave on the BG horizontal offset for scanlines
 * [start, start + length); the wave grows with depth up to the
 * amplitude step count. Other lines keep the BG offset.
 */
void BgEffect_DeepeningHWave(s32 bgNum, s32 top, s32 height, s32 depth)
{
    u32 frameCtr;
    struct BgLayerState *bg;
    s32 level;
    s32 scanline;
    s32 bottom;
    struct BgOffset *bgOfs;
    u16 *hBlankBuf;

    frameCtr = gEwramData->frameCounter;
    bottom = top + height;
    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];

    bgOfs = &gDisplayRegisters.bgOfs[bgNum];
    bg = &gEwramData->bgInfo[bgNum];

    if (bottom > SCREEN_SIZE_Y)
    {
        bottom = SCREEN_SIZE_Y;
    }
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0HOFS + bgNum);

    for (scanline = 0; scanline < top; scanline++)
    {
        *hBlankBuf++ = bgOfs->hOfs;
    }

    for (scanline = top; scanline < bottom; scanline++)
    {
        level = (depth * (scanline - top)) / height;
        *hBlankBuf++ = (Sin(((bg->yPos.part16.integer + scanline) << 13) + (frameCtr << (10 - level))) >> ((level >> 1) + 15)) + bgOfs->hOfs;
    }

    for (; scanline < SCREEN_SIZE_Y; scanline++)
    {
        *hBlankBuf++ = bgOfs->hOfs;
    }
    
    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief H-blank sine wave of the given amplitude on the BG horizontal
 * offset between map rows top..bottom (clipped to the screen),
 * animated by the frame counter.
 */
void BgEffect_HWaveInRange(s32 bgNum, s32 top, s32 bottom, s32 amplitude)
{
    s32 height;
    s32 scanline;
    struct BgOffset *bgOfs;
    u16 *hBlankBuf;
    s32 frameCtr;
    struct BgLayerState *bgInfo;
    s32 wave;

    frameCtr = gEwramData->frameCounter;
    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];

    bgOfs = &gDisplayRegisters.bgOfs[bgNum];
    bgInfo = &gEwramData->bgInfo[bgNum];

    height = bottom - top;
    top = top - bgInfo->yPos.part16.integer;
    if (top < 0)
    {
        top = 0;
    }
    else if (top > SCREEN_SIZE_Y)
    {
        top = SCREEN_SIZE_Y;
    }

    if ((top + height) > SCREEN_SIZE_Y)
    {
        height = SCREEN_SIZE_Y - top;
    }

    bottom = top + height;
    if (bottom > SCREEN_SIZE_Y)
    {
        bottom = SCREEN_SIZE_Y;
    }
    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0HOFS + bgNum);

    for (scanline = 0; scanline < top; scanline++)
    {
        *hBlankBuf++ = bgOfs->hOfs;
    }
    frameCtr = frameCtr << 9;

    for (scanline = top; scanline < bottom; scanline++)
    {
        wave = Sin(((bgInfo->yPos.part16.integer + scanline) << 0xC) + (frameCtr));
        *hBlankBuf++ = ((amplitude * wave) >> 0x10) + bgOfs->hOfs;
    }

    for (; scanline < SCREEN_SIZE_Y; scanline++)
    {
        *hBlankBuf++ = bgOfs->hOfs;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief H-blank horizontal skew growing linearly between map rows
 * top..bottom, modulated by a sine of the frame counter (slope 8).
 */
void BgEffect_SkewWave(s32 bgNum, s32 top, s32 bottom)
{
    u32 frameCtr;
    s32 baseWave;
    s16 *hBlankBuf;
    s32 scanline;
    s32 height;
    struct BgOffset *bgOfs;
    struct BgLayerState *bgInfo;
    s32 wave;
    s32 offset;

    frameCtr = gEwramData->frameCounter;
    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    bgOfs = &gDisplayRegisters.bgOfs[bgNum];
    bgInfo = &gEwramData->bgInfo[bgNum];

    height = bottom - top;
    top = top - bgInfo->yPos.part16.integer;
    if (top < 0)
    {
        top = 0;
    }
    else if (top > SCREEN_SIZE_Y)
    {
        top = SCREEN_SIZE_Y;
    }
    if ((top + height) > SCREEN_SIZE_Y)
    {
        height = SCREEN_SIZE_Y - top;
    }
    bottom = top + height;

    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0HOFS + bgNum);
    baseWave = Sin(frameCtr << 0xC);

    for (scanline = 0; scanline < SCREEN_SIZE_Y; scanline++)
    {
        if ((scanline < top) || (scanline > bottom))
        {
            *hBlankBuf = bgOfs->hOfs;
        }
        else
        {
            wave = Sin((s32) (((-(top << 0xC)) + 0x1000 * scanline) + (frameCtr << 0xD)) >> 2);
            offset = 8 * scanline + top * -8;
            *hBlankBuf = bgOfs->hOfs + ((offset - ((offset * (wave + baseWave)) >> 0x13)) / height);
        }
        hBlankBuf++;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief Like BgEffect_SkewWave with slope 12, animated by the play timer.
 */
void BgEffect_SkewWaveSteep(s32 bgNum, s32 top, s32 bottom)
{
    s32 baseWave;
    s16 *hBlankBuf;
    s32 scanline;
    s32 height;
    struct BgOffset *bgOfs;
    s32 phase;
    struct BgLayerState *bgInfo;
    s32 offset;
    s32 angle;
    s32 wave;
    s32 skew;

    phase = gEwramData->gameState.bgEffectFrameCounter;
    hBlankBuf = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    bgOfs = &gDisplayRegisters.bgOfs[bgNum];
    bgInfo = &gEwramData->bgInfo[bgNum];

    height = bottom - top;
    top = top - bgInfo->yPos.part16.integer;
    if (top < 0)
    {
        top = 0;
    }
    else if (top > SCREEN_SIZE_Y)
    {
        top = SCREEN_SIZE_Y;
    }
    if ((top + height) > SCREEN_SIZE_Y)
    {
        height = SCREEN_SIZE_Y - top;
    }
    bottom = top + height;

    HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0HOFS + bgNum);
    phase = phase << 9;
    baseWave = Sin(phase);

    for (scanline = 0; scanline < SCREEN_SIZE_Y; scanline++)
    {
        if ((scanline < top) || (scanline > bottom))
        {
            *hBlankBuf = bgOfs->hOfs;
        }
        else
        {
            offset = (scanline - top) * 0xC;
            angle = (scanline - top) << 0xC;
            wave = Sin((phase + angle) >> 2);
            skew = (offset * (wave + baseWave)) >> 0x13;
            *hBlankBuf = bgOfs->hOfs + ((offset - skew) / height);
        }
        *hBlankBuf++;
    }

    gEwramData->hBlankEffect.requestStart = 1;
}

/**
 * @brief BG scroll mode 18: H-blank BG2 affine parameters giving a scrolling
 * ceiling (upper 128 lines) and floor perspective.
 */
void BgEffect_Bg2Perspective(void)
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

/**
 * @brief Set the BG2 affine transform around (x, y): rotate by
 * play timer * speed and pulse the horizontal scale.
 */
void BgEffect_Bg2RotateAndPulse(s32 scrX, s32 scrY, s32 rotateSpeed)
{
    u32 timer;
    struct BgAffineSrcData src;

    timer = gEwramData->gameState.bgEffectFrameCounter;
    src.texX = 0x8000;
    src.texY = 0x8000;
    src.scrX = scrX;
    src.scrY = scrY;
    src.sx = (Sin(timer << 7) >> 8) + 0x100;
    src.sy = 0x100;
    src.alpha = (timer * rotateSpeed) << 7;
    BgAffineSet(&src, (struct BgAffineDstData *) &gDisplayRegisters.bg2PA, 1);
}

/**
 * @brief Save display control, blend settings, H-blank state, BG control and
 * window registers in gameState.unk_68/unk_78 before a menu screen.
 */
void Display_SaveState(void)
{
    struct GameState *gameState;
    struct SavedDisplayState *saved;

    gameState = &gEwramData->gameState;
    saved = &gameState->savedDisplay;

    saved->displayControl = gUnk_03002CB0.dispCnt;
    saved->blendControl = gDisplayRegisters.bldCnt;
    saved->blendAlpha = gDisplayRegisters.bldAlpha;
    saved->vcountDmaMode = gUnk_03002CB0.vcountDmaMode;
    saved->hBlankRequestStart = gEwramData->hBlankEffect.requestStart;
    saved->hBlankVcountSetting = gEwramData->hBlankEffect.vcountSetting;

    DMA_COPY_32(3, &gDisplayRegisters.bgCnt, &gameState->savedDisplay.bgControlBytes, sizeof(gDisplayRegisters.bgCnt));
    DMA_COPY_32(3, &gDisplayRegisters.win0H, &gameState->savedWindowRegisters, sizeof(gameState->savedWindowRegisters));
}

/**
 * @brief Restore the display state saved by Display_SaveState.
 */
void Display_RestoreState(void)
{
    struct GameState *gameState;
    struct SavedDisplayState *saved;

    gameState = &gEwramData->gameState;
    saved = &gameState->savedDisplay;

    gUnk_03002CB0.dispCnt = saved->displayControl;
    gDisplayRegisters.bldCnt = saved->blendControl;
    gDisplayRegisters.bldAlpha = saved->blendAlpha;
    gUnk_03002CB0.vcountDmaMode = saved->vcountDmaMode;
    gEwramData->hBlankEffect.requestStart = saved->hBlankRequestStart;
    gEwramData->hBlankEffect.vcountSetting = saved->hBlankVcountSetting;
    
    DMA_COPY_32(3, &gameState->savedDisplay.bgControlBytes, &gDisplayRegisters.bgCnt, sizeof(gameState->savedDisplay.bgControlBytes));
    DMA_COPY_32(3, &gameState->savedWindowRegisters, &gDisplayRegisters.win0H, sizeof(gameState->savedWindowRegisters));
}

static inline s32 Bg_ScaleScroll(s32 value, s32 range, s32 scale, s32 valueOffset, s32 rangeOffset)
{
    return (scale * (value - valueOffset)) / (range - rangeOffset);
}

/**
 * @brief Per-frame BG update: apply the scroll mode (bgInfo unk_18) of BG1
 * (mode 0x1A) and BG2/BG3 - parallax ratios, autoscroll, H-blank
 * waves and affine effects - from the BG1 scroll velocity.
 */
void Bg_UpdateScrollModes(void)
{
    struct BgLayerState *bg;
    s32 scrollX;
    s32 scrollY;
    s32 waveShift;
    u16 *waveBase;
    s16 *hblank;
    s16 *waveLine;
    s16 *swayLine;
    s32 sway;
    s32 i;
    s32 waveFreq;
    s32 k;
    s32 rippleFreq;
    s32 direction;
    s32 rippleShift;
    s32 j;
    s32 centerY;
    s32 line;
    s32 centerX;
    s32 bgNum;
    struct BgOffset *ofs;
    u16 *vOfs;
    u32 spin;
    s32 step;
    u32 wavePhase;
    u32 ripplePhase;
    struct BgLayerState *layer;
    struct BgAffineSrcData src;
    struct BgAffineSrcData src1;
    u16 *swayBase;
    u16 *rippleBase;

    layer = bg = &gEwramData->bgInfo[1];
    scrollX = bg->xVel.whole;
    scrollY = bg->yVel.whole;
    if (bg->scrollMode == 0x1A)
    {
        BgEffect_SkewWave(1, 0x450, 0x4A0);
    }

    for (bgNum = 2; bgNum < 4; bgNum++)
    {
        layer = &gEwramData->bgInfo[bgNum];
        ofs = &gDisplayRegisters.bgOfs[bgNum];
        vOfs = &gDisplayRegisters.bgOfs[bgNum].vOfs;
        switch (layer->scrollMode)
        {
            case 1:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY);
                break;

            case 5:
                BgMap_ScrollWithMargin(bgNum, scrollX * 2, scrollY);
                break;

            case 6:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY * 2);
                break;

            case 7:
                BgMap_ScrollWithMargin(bgNum, scrollX * 2, scrollY * 2);
                break;

            case 2:
                BgMap_ScrollWithMargin(bgNum, scrollX >> 1, scrollY);
                break;

            case 3:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY >> 1);
                break;

            case 4:
                BgMap_ScrollWithMargin(bgNum, scrollX >> 1, scrollY >> 1);
                break;

            case 8:
                ofs->hOfs += 1;
                break;

            case 9:
                ofs->hOfs += ~scrollX;
                break;

            case 10:
                if ((gEwramData->gameState.gameFlagsTail[GAME_FLAG_WORD(FLAG_JULIUS_ROOM_CLEARED) - 2] & GAME_FLAG_BIT(FLAG_JULIUS_ROOM_CLEARED)) != 0)
                {
                    *vOfs -= 2;
                }
                break;

            case 11:
                BgMap_ScrollWithMargin(bgNum, Bg_ScaleScroll(layer->pixelWidth, bg->pixelWidth, scrollX, 0xF0, 0xF0), scrollY);
                break;

            case 12:
                BgMap_ScrollWithMargin(bgNum, scrollX, Bg_ScaleScroll(layer->pixelHeight, bg->pixelHeight, scrollY, 0xD0, 0xD0));
                break;

            case 13:
                BgMap_ScrollWithMargin(bgNum, Bg_ScaleScroll(layer->pixelWidth, bg->pixelWidth, scrollX, 0xF0, 0xF0), Bg_ScaleScroll(layer->pixelHeight, bg->pixelHeight, scrollY, 0xD0, 0xD0));
                break;

            case 14:
                BgEffect_Bg3ScanlineParallax();
                break;

            case 15:
                BgMap_ScrollWithMargin(bgNum, Bg_ScaleScroll(layer->pixelWidth, bg->pixelWidth, scrollX, 0xF0, 0xF0), Bg_ScaleScroll(layer->pixelHeight, bg->pixelHeight, scrollY, 0xD0, 0xD0));
                BgEffect_DeepeningHWave(bgNum, 0, SCREEN_SIZE_Y, 1);
                break;

            case 16:
                BgMap_ScrollWithMargin(bgNum, Bg_ScaleScroll(layer->pixelWidth, bg->pixelWidth, scrollX, 0xF0, 0xF0), Bg_ScaleScroll(layer->pixelHeight, bg->pixelHeight, scrollY, 0xD0, 0xD0));
                BgEffect_HWaveInRange(bgNum, 0, SCREEN_SIZE_Y, 2);
                break;

            case 18:
                BgEffect_Bg2Perspective();
                break;

            case 19:
                if (!(gEwramData->gameState.bossRushMode) && (gEwramData->gameState.gameFlagsTail[GAME_FLAG_WORD(FLAG_HEADHUNTER_ROOM_CLEARED) - 2] & GAME_FLAG_BIT(FLAG_HEADHUNTER_ROOM_CLEARED)))
                {
                    gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
                }
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY);
                break;

            case 20:
                step = gEwramData->gameState.bgEffectFrameCounter;
                hblank = (s16*)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
                HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, REG_BG3HOFS);

                for (i = 0; i < 0x80; i++)
                {
                    *hblank++ = 0;
                }

                for (i = 0; i < 8; i++)
                {
                    *hblank++ = step;
                }

                for (i = 0; i < 8; i++)
                {
                    *hblank++ = (step * 3) >> 1;
                }

                for (i = 0; i < 0x14; i++)
                {
                    *hblank++ = step * 2;
                }

                gEwramData->hBlankEffect.requestStart = 1;
                break;

            case 21:
                if (gEwramData->gameState.gameFlagsTail[GAME_FLAG_WORD(FLAG_JULIUS_ROOM_CLEARED) - 2] & GAME_FLAG_BIT(FLAG_JULIUS_ROOM_CLEARED))
                {
                    src.texX = 0x8000;
                    src.texY = 0x8000;
                    src.scrX = 0x70;
                    src.scrY = 0x38;
                    src.sx = 0x80;
                    src.sy = 0x80;
                    src.alpha = -gEwramData->gameState.bgEffectFrameCounter << 9;
                    BgAffineSet(&src, (struct BgAffineDstData *) &gDisplayRegisters.bg2PA, 1);
                }
                else
                {
                    gUnk_03002CB0.dispCnt &= ~DCNT_BG2;
                }
                break;

            case 23:
                BgMap_ScrollWithMargin(bgNum, Bg_ScaleScroll(layer->pixelWidth, bg->pixelWidth, scrollX, 0xF0, 0xF0), scrollY);
                BgEffect_DeepeningHWave(bgNum, (s32)(SCREEN_SIZE_Y * 0.7f), (s32)(SCREEN_SIZE_Y * 0.3f), 3);
                break;

            case 24:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY);
                BgEffect_HWaveInRange(bgNum, 0xB0, 0x200, 1);
                break;

            case 25:
                BgEffect_HWaveInRange(bgNum, 0x70, 0x100, 1);
                break;

            case 26:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY);
                break;

            case 27:
                spin = gEwramData->gameState.bgEffectFrameCounter;
                src1.texX = 0x8000;
                src1.texY = 0x8000;
                src1.scrX = 0x100 - bg->xPos.part16.integer;
                src1.scrY = 0x100 - bg->yPos.part16.integer;
                src1.sy = src1.sx = ((Sin(spin * 0x10) * 3) >> 0xD) + 0x58;
                src1.alpha = -spin << 7;
                BgAffineSet(&src1, (struct BgAffineDstData *) &gDisplayRegisters.bg2PA, 1);
                break;

            case 28:
                BgEffect_SkewWaveSteep(bgNum, 0x38, 0x90);
                break;

            case 29:
                BgEffect_SkewWaveSteep(bgNum, 0x70, 0xCC);
                break;

            case 30:
                waveShift = 1;
                wavePhase = gEwramData->gameState.bgEffectFrameCounter;
                waveBase = &gDisplayRegisters.bgOfs[bgNum].vOfs;
                waveLine = (s16*)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
                HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0VOFS + bgNum);

                waveShift = 0xD;
                wavePhase <<= 0xA;
                waveFreq = 0xB;
                for (j = 0; j < SCREEN_SIZE_Y; j++)
                {
                    *waveLine++ = (Sin((j << waveFreq) + wavePhase) >> waveShift) + *waveBase;
                }

                gEwramData->hBlankEffect.requestStart = 1;
                layer->xPos.whole += scrollX;
                ofs->hOfs = layer->xPos.part16.integer;
                break;

            case 31:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY);
                swayLine = (s16*)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
                swayBase = &gDisplayRegisters.bgOfs[bgNum].hOfs;
                HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0HOFS + bgNum);

                sway = Sin(gEwramData->frameCounter << 8) >> 0xF;
                for (k = 0; k < SCREEN_SIZE_Y; k++)
                {
                    *swayLine++ = *swayBase + sway;
                }

                gEwramData->hBlankEffect.requestStart = 1;
                break;

            case 32:
                centerX = 0x78;
                centerY = 0x50;
                direction = 1;
                
                switch (GetRoomFromMapPosition(GetEntityRoomXPositionInteger(gEwramData->playerState.player), GetEntityRoomYPositionInteger(gEwramData->playerState.player)))
                {
                    case 0:
                        centerX = 0xD0;
                        centerY = 0x30;
                        direction = 1;
                        break;

                    case 16:
                        centerX = 0x78;
                        centerY = 0x78;
                        direction = 1;
                        break;

                    case 33:
                        centerX = 0x20;
                        centerY = 0x68;
                        direction = -1;
                        break;

                    case 34:
                        centerX = 0x20;
                        centerY = 0x50;
                        direction = -1;
                        break;

                    case 35:
                        centerX = 0x20;
                        centerY = 0x50;
                        direction = -1;
                        break;

                    case 39:
                        centerX = 0x20;
                        centerY = 0x10;
                        direction = -1;
                        break;
                }
                BgEffect_Bg2RotateAndPulse(centerX, centerY, direction);
                break;

            case 36:
                BgMap_ScrollWithMargin(bgNum, scrollX, scrollY);
                ripplePhase = gEwramData->gameState.bgEffectFrameCounter;
                rippleBase = &gDisplayRegisters.bgOfs[bgNum].vOfs;
                waveLine = (s16*)&gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
                HBlankEffectSetup(0, SCREEN_SIZE_Y, 2, (struct BgOffset *)REG_BG0VOFS + bgNum);

                rippleShift = 0xE;
                ripplePhase <<= 0xA;
                rippleFreq = 0xC;
                for (line = 0; line < 160; line++)
                {
                    *waveLine++ = (Sin((line << rippleFreq) + ripplePhase) >> rippleShift) + *rippleBase;
                }

                gEwramData->hBlankEffect.requestStart = 1;
                break;
        }
    }
}

/**
 * @brief Set up BG0 as the HUD/text layer (font graphics, text buffer and
 * colors), draw the gauge frames, copy current/max HP and MP into the
 * displayed values and draw both gauges.
 */
void Hud_Init(void)
{
    struct PlayerStatus *status;
    struct GameState *gameState;

    DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    DMA_FILL_32(3, 0, VRAM_BASE, 0x20);

    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 6;

    Text_SetTileBuffer(VRAM_BASE + 0x5000);
    Text8_SetRegion(0, 0, 0x1A, 1);
    Text_SetForegroundColor(0xF);
    Text_SetBackgroundColor(1);
    Gfx_CopyResource((u8 *)sHudGfx, VRAM_BASE + 0x4000, 0);

    DMA_FILL_32(3, 0, VRAM_BASE + 0x5800, 0x1954);
    DMA_FILL_32(3, 0, VRAM_BASE + 0x5000, 0x2000);

    BgPalette_LoadRange((u8 *)sSavePromptBgPalette, 0, 2, 0xD);
    Hud_DrawGaugeFrames();

    status = &gEwramData->playerStatus;
    gameState = &gEwramData->gameState;

    gameState->hudHP = status->currentHP;
    gameState->hudMP = status->currentMP;
    gameState->hudMaxHP = status->maxHP;
    gameState->hudMaxMP = status->maxMP;

    Hud_DrawGauge(4, 1, gameState->hudHP, status->maxHP, 0);
    Hud_DrawGauge(4, 2, gameState->hudMP, status->maxMP, 1);
}

struct HudGaugeFrameBlock {
    s32 destinationX;
    s32 destinationY;
    s32 sourceTileX;
    s32 sourceTileY;
    s32 width;
    s32 height;
};

const struct HudGaugeFrameBlock sHudGaugeFrameBlocks[2] = {
    [0] = {
        .destinationX = 0,
        .destinationY = 0,
        .sourceTileX = 9,
        .sourceTileY = 0,
        .width = 4,
        .height = 5
    },
    [1] = {
        .destinationX = 12,
        .destinationY = 0,
        .sourceTileX = 13,
        .sourceTileY = 0,
        .width = 2,
        .height = 4
    }
};

// #define VRAM_E000 ((u16 (*)[0x20])(VRAM_BASE + 0xE000))

/**
 * @brief Write the two gauge frame tile blocks described by sHudGaugeFrameBlocks
 * into the BG0 tilemap with palette 13.
 */
void Hud_DrawGaugeFrames(void)
{
    s32 i;
    s32 j;
    s32 k;
    s32 x;
    s32 y;
    s32 tileX;
    s32 tileY;
    s32 palette = 0xD;
    struct HudGaugeFrameBlock unk_080E0F74[2];

    memcpy(unk_080E0F74, sHudGaugeFrameBlocks, sizeof(unk_080E0F74));

    for (i = 0; i < 2; i++)
    {
        x = unk_080E0F74[i].destinationX;
        y = unk_080E0F74[i].destinationY;
        tileX = unk_080E0F74[i].sourceTileX;
        tileY = unk_080E0F74[i].sourceTileY;
        for (j = 0; j < unk_080E0F74[i].height; j++, y++)
        {
            for (k = 0; k < unk_080E0F74[i].width; k++, x++)
            {
                *((u16*)(VRAM_BASE + 0xE000) + x + y * 0x20) = (0x200 + k + tileX + (tileY + j) * 0x10) | (palette << 12);
                // VRAM_E000[var_1][var_0] = (0x200 + k + var_2 + (var_3 + j) * 0x10) | (palette << 12);
            }
            x = unk_080E0F74[i].destinationX;
        }
    }
}

/* Draw an 8-tile gauge at (x, y) in BG map 0x0600E000 filled to value/max. */
static inline void DrawStatGauge(s32 x, s32 y, s32 value, s32 max, s32 isMp)
{
    s32 i;
    u16 *dst;
    s32 fill;
    s32 tileBase;
    s32 palette;

    palette = 0xD;
    if (isMp == 0)
        tileBase = 0x200;
    else
        tileBase = 0x210;

    if (value > 0)
    {
        fill = Div(Div(value * 100, max) << 6, 100);
        if (fill == 0)
            fill = 1;
    }
    else
    {
        fill = 0;
    }

    dst = (u16 *)(VRAM_BASE + 0xE000 + (x * 2) + (y << 6));
    for (i = 0; i < 8; i++)
    {
        if (i == fill / 8)
            *dst = (tileBase + (fill - (i * 8))) | (palette << 12);
        else if (fill >= (i + 1) * 8)
            *dst = (tileBase + 8) | (palette << 12);
        else
            *dst = tileBase | (palette << 12);
        dst += 1;
    }
}

// fakematch: https://decomp.me/scratch/6f0Up
/* Move the displayed HP/MP values one step per iteration towards the real
 * player stats and redraw the gauge when anything changed. */
/**
 * @brief Move the displayed HP and MP (and their maxima) toward the player
 * stats by a few points per update and redraw the changed gauge.
 */
void Hud_UpdateHpMpGauges(struct GameState *gameState)
{
    struct PlayerStatus *stats;
    s32 changed;
    s32 speed;
    s32 i, j;
    s16 *hp, *maxHp, *mp, *maxMp;

    stats = &gEwramData->playerStatus;
    changed = 0;
    speed = 2;
    hp = &gameState->hudHP;
    maxHp = &gameState->hudMaxHP;
    #ifndef NON_MATCHING
    asm("" ::: "r5"); /* FAKEMATCH: keeps maxHp out of r5 (the counter gets it) */
    #endif
    for (i = 0; i < speed; i++)
    {
        if (*hp != stats->currentHP)
        {
            s32 value = *hp;
            s16 target = stats->currentHP;
            if (*hp < target)
            {
                *hp = value + 1;
                changed = 1;
            }
            else
            {
                *hp = value - 1;
                changed = 1;
            }
        }
        if (*maxHp != stats->maxHP)
        {
            s32 value = *maxHp;
            if (*maxHp < stats->maxHP)
            {
                *maxHp = value + 1;
                changed = 1;
            }
            else
            {
                *maxHp = value - 1;
                changed = 1;
            }
        }
    }
    if (changed)
        DrawStatGauge(4, 1, gameState->hudHP, gameState->hudMaxHP, 0);

    changed = 0;
    #ifndef NON_MATCHING
    asm("" ::: "r4"); /* FAKEMATCH: restores the r4/r5 assignment of the second loop */
    #endif
    for (j = 0; j < speed; j++)
    {
        mp = &gameState->hudMP;
        maxMp = &gameState->hudMaxMP;
        if (*mp != stats->currentMP)
        {
            s32 value = *mp;
            s16 target = stats->currentMP;
            if (*mp < target)
            {
                *mp = value + 1;
                changed = 1;
            }
            else
            {
                *mp = value - 1;
                changed = 1;
            }
        }
        if (*maxMp != stats->maxMP)
        {
            s32 value = *maxMp;
            if (*maxMp < stats->maxMP)
            {
                *maxMp = value + 1;
                changed = 1;
            }
            else
            {
                *maxMp = value - 1;
                changed = 1;
            }
        }
    }
    if (changed)
        DrawStatGauge(4, 2, gameState->hudMP, gameState->hudMaxMP, 1);
}

/**
 * @brief Draw the enemy name window for the text id and start its
 * 0xF0-update display timer, or keep the request pending while the
 * DMA queue is full.
 */
void Hud_ShowEnemyName(s32 hudMessageId)
{
    u16 *dst;
    u8* text;
    s32 textWidth;
    s32 i;
    struct GameState *gameState;
    u8 bufferIndex;
    s32 row;
    s32 width;

    gameState = &gEwramData->gameState;
    bufferIndex = gEwramData->gameState.hudTextBufferIndex;

    text = Text_GetLocalizedEntry(hudMessageId);
    Text_SetBackgroundColor(1);
    Text8_SetPixelCursor(0, 0);

    if (bufferIndex == 0)
    {
        Text8_SetRegion(0, 0, 0x14, 1);
        Text8_SetCursor(0, 0);
    }
    else
    {
        Text8_SetRegion(0, 1, 0x14, 1);
        Text8_SetCursor(0, 1);
    }

    row = 0x12;
    Text8_Print((struct unk_08506B38 *) text, 0);
    textWidth = (Text8_GetPixelX() + 7) >> 3;
    Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(0x35E), 0);
    width = textWidth;
    dst = (u16*)&gEwramData->sharedScratchBuffer;

    if (gEwramData->gameState.stateFlags & (STATE_HUD_MESSAGES_BLOCKED | STATE_LARGE_WINDOW_OPEN | STATE_MESSAGE_WINDOW_CLOSED))
    {
        gameState->hudMessageRequest = 0;
        return;
    }
    Hud_CloseMessageWindow();

    for (i = 0x1A - width; i > 0; i--)
    {
        *dst++ = 0;
    }

    *dst++ = 0xE220;
    *dst++ = 0xE221;
    *dst++ = 0xE222;

    for (i = width - 2; i > 0; i--)
    {
        *dst++ = 0xE223;
    }

    *dst++ = 0xE622;
    *dst++ = 0xE621;
    *dst++ = 0xE620;

    for (i = 0x1C - width; i > 0; i--)
    {
        *dst++ = 0;
    }

    *dst++ = 0xE230;
    *dst++ = 0xE231;

    for (i = 0; i < width; i++)
    {
        *dst++ = (i + (bufferIndex << 5) + 0x80) | 0xE200;
    }

    *dst++ = 0xE631;
    *dst++ = 0xE630;

    for (i = 0x1C - width; i > 0; i--)
    {
        *dst++ = 0;
    }

    *dst++ = 0xE240;
    *dst++ = 0xE241;
    *dst++ = 0xE242;

    for (i = width - 2; i > 0; i--)
    {
        *dst++ = 0xE243;
    }

    *dst++ = 0xE642;
    *dst++ = 0xE641;
    *dst++ = 0xE640;

    if (DmaQueue_DirectCopy(0xC0, (u32 *)gEwramData->sharedScratchBuffer, (u32 *)(VRAM_BASE + 0xE000 + row * 0x40)) != NULL)
    {
        BgPalette_LoadRange((u8 *)sSavePromptBgPalette, 5, 1, 0xE);
        BgPalette_LoadRange((u8 *)sTextBgPalette, 1, 1, 0xF);

        gameState->hudMessageTimer = 0xF0;
        gameState->hudMessageRequest = 0;
        gameState->hudTextBufferIndex = 1 - gameState->hudTextBufferIndex;
    }
    else
    {
        gameState->hudMessageRequest = 2;
    }
    gameState->hudMessageId = hudMessageId;
}

/**
 * @brief Draw the pickup message window for the text id and start its
 * 0xF0-update display timer, or keep the request pending while the
 * DMA queue is full.
 */
void Hud_ShowPickupMessage(s32 hudMessageId)
{
    u8 *text;
    s32 textWidth;
    s32 i;
    struct GameState *gameState;
    u16 *dst;
    u8 bufferIndex;
    s32 palette;
    s32 row;
    s32 width;

    gameState = &gEwramData->gameState;
    bufferIndex = gEwramData->gameState.hudTextBufferIndex;
    dst = (u16*)&gEwramData->sharedScratchBuffer;
    palette = 4;
    if (gEwramData->gameState.stateFlags & (STATE_HUD_MESSAGES_BLOCKED | STATE_LARGE_WINDOW_OPEN | STATE_MESSAGE_WINDOW_CLOSED))
    {
        gEwramData->gameState.hudMessageRequest = 0;
        return;
    }
    text = Text_GetLocalizedEntry(hudMessageId);
    Text_SetBackgroundColor(1);
    Text8_SetPixelCursor(0, 0);
    if (bufferIndex == 0)
    {
        Text8_SetRegion(0, 0, 0x14, 1);
        Text8_SetCursor(0, 0);
    }
    else
    {
        Text8_SetRegion(0, 1, 0x14, 1);
        Text8_SetCursor(0, 1);
    }
    row = 0x12;
    Text8_Print((struct unk_08506B38 *) text, 0);
    textWidth = (Text8_GetPixelX() + 7) >> 3;
    Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(0x35E), 0);
    width = textWidth;
    AreaName_Dismiss(&gEwramData->gameState);
    Hud_CloseMessageWindow();

    *dst++ = 0xE220;
    if (palette == 4)
    {
        *dst++ = 0xE221;
        *dst++ = 0xE222;
    }
    else
    {
        *dst++ = 0xE234;
        *dst++ = 0xE235;
    }

    for (i = width - 2; i > 0; i--)
    {
        *dst++ = 0xE223;
    }

    *dst++ = 0xE622;
    *dst++ = 0xE621;
    *dst++ = 0xE620;

    for (i = 0x1C - width; i > 0; i--)
    {
        *dst++ = 0;
    }

    *dst++ = 0xE230;
    *dst++ = 0xE231;

    for (i = 0; i < width; i++)
    {
        *dst++ = (i + (bufferIndex << 5) + 0x80) | 0xE200;
    }

    *dst++ = 0xE631;
    *dst++ = 0xE630;

    for (i = 0x1C - width; i > 0; i--)
    {
        *dst++ = 0;
    }

    *dst++ = 0xE240;
    if (palette == 4)
    {
        *dst++ = 0xE241;
        *dst++ = 0xE242;
    }
    else
    {
        *dst++ = 0xE254;
        *dst++ = 0xE255;
    }

    for (i = (width - 2); i > 0; i--)
    {
        *dst++ = 0xE243;
    }

    *dst++ = 0xE642;
    *dst++ = 0xE641;
    *dst++ = 0xE640;

    if (DmaQueue_DirectCopy(0xC0, (u32 *)gEwramData->sharedScratchBuffer, (u32 *)(VRAM_BASE + 0xE000 + row * 0x40)) != NULL)
    {
        BgPalette_LoadRange((u8 *)sSavePromptBgPalette, palette, 1, 0xE);
        BgPalette_LoadRange((u8 *)sTextBgPalette, 1, 1, 0xF);

        gameState->hudMessageTimer = 0xF0;
        gameState->hudMessageRequest = 0;
        gameState->hudTextBufferIndex = 1 - gameState->hudTextBufferIndex;
    }
    else
    {
        gameState->hudMessageRequest = 1;
    }
    gameState->hudMessageId = hudMessageId;
}

// #define VRAM_5C00 ((u32 (*)[0x10][8])(VRAM_BASE + 0x5C00))

/**
 * @brief Fill the large message window text tiles (VRAM 0x06005C00) with
 * colour 2 and draw its two border tiles at 0x06005600/0x06005620.
 */
void Hud_ClearLargeWindowTiles(void)
{
    s32 j;
    s32 k;
    s32 i;
    s32 fillWord;
    s32 color;

    color = 2;
    fillWord = 0;

    for (i = 0; i < 8; i++)
    {
        fillWord |= color << (i * 4);
    }

    for (j = 0; j < 5; j++)
    {
        for (k = 0; k < 0x1E; k++)
        {
            for (i = 0; i < 8; i++)
            {
                *(u32*)((u32*)(VRAM_BASE + 0x5C00) + (j << 7) + (k << 3) + 1 * i) = fillWord;
                // VRAM_5C00[j][k][i] = var_r5;
            }
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (i == 0)
        {
            *(s32* )(VRAM_BASE + 0x5600) = -1;
        }
        else
        {
            *((u32* )(VRAM_BASE + 0x5600) + i) = fillWord;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (i == 7)
        {
            *((u32* )(VRAM_BASE + 0x5620) + i) = -1;
        }
        else
        {
            *((u32* )(VRAM_BASE + 0x5620) + i) = fillWord;
        }
    }
}

static inline s32 Hud_IsWindowAreaFree()
{
    s32 res;
    res = 1;
    if ((gEwramData->gameState.stateFlags & STATE_EVENT_RUNNING) && (gEwramData->playerState.player->posY.part16.integer > 0x70))
        res = 0;
    return res;
}

/**
 * @brief Draw the two prompt tiles at the window edge: blinking with the
 * play timer while the large window is open (stateFlags bit 0x01000000),
 * static otherwise; the row depends on the player height check.
 */
void Hud_DrawWindowPrompt(void)
{
    s32 row;
    u8 *buffer;
    s32 column;
    u16 *tile;

    buffer = gEwramData->sharedScratchBuffer;
    column = 0x1C;
    
    if (Hud_IsWindowAreaFree() != 0)
    {
        row = 0x11;
    }
    else
    {
        row = 8;
    }

    if (!(gEwramData->gameState.stateFlags & STATE_LARGE_WINDOW_OPEN))
    {
        tile = ((u16*)gEwramData->sharedScratchBuffer);
        *tile = 0xF228;
        DmaQueue_DirectCopy(2, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (row * 0x40)));
        row++;

        tile = ((u16*)gEwramData->sharedScratchBuffer);
        *tile = 0xF238;
        DmaQueue_DirectCopy(2, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (row * 0x40)));
    }
    else
    {
        tile = ((u16*)gEwramData->sharedScratchBuffer);
        *tile = (gEwramData->gameState.inGameTimer & 8) ? 0xF226 : 0xF227;
        DmaQueue_DirectCopy(2, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (row * 0x40)));
        row++;

        tile = ((u16*)gEwramData->sharedScratchBuffer);
        *tile = (gEwramData->gameState.inGameTimer & 8) ? 0xF236 : 0xF237;
        DmaQueue_DirectCopy(2, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (row * 0x40)));
    }
}

/**
 * @brief Open the large message window (stateFlags bit 0x01000000) and print
 * menu text base[kind] + n - 1 (n > 0) or text 0x361 (n == 0),
 * then draw the window frame rows.
 */
void Hud_ShowLargeSoulWindow(s32 soulType, s32 soulId)
{
    s32 j;
    s32 row;
    s32 i;
    s32 index;
    u8 *buffer;
    u16 *dst;
    s32 width;
    s32 firstRow;
    s32 copyRow;
    s32 column;
    s32 textX;
    s32 textY;

    textX = 7;
    textY = 2;
    column = 0;

    index = 0x361;
    gEwramData->gameState.stateFlags |= STATE_LARGE_WINDOW_OPEN;
    BgPalette_LoadRange((u8 *)sSavePromptBgPalette, 7, 1, 0xF);

    if (soulId != 0)
    {
        switch (soulType)
        {
            case 0:
                index = 0x1E4;
                break;
            case 1:
                index = 0x21C;
                break;
            case 2:
                index = 0x234;
                break;
            case 3:
                index = 0x257;
                break;
        }
        index -= 1 - soulId;
    }

    Text_SetForegroundColor(0xF);
    Text_SetBackgroundColor(2);
    Text12_SetRegion(textX, textY, 0x1E, 2);
    Text12_SetCursor(0, 2);
    Text12_ClearRegion();
    Hud_ClearLargeWindowTiles();
    MenuText_Print(index);
    Text12_SetCursor(textX, textY);
    Text_SetBackgroundColor(1);

    width = 0x1E;
    buffer = gEwramData->sharedScratchBuffer;
    
    if (Hud_IsWindowAreaFree() == 0)
    {
        row = 7;
    }
    else
    {
        row = 0x10;
    }

    firstRow = row;
    for (i = 0; i < 3; i++)
    {
        dst = (u16*)&gEwramData->sharedScratchBuffer;
        copyRow = row;
        for (j = 0; j < width; j++)
        {
            *dst++ = j + (i << 5) + 0xE2E0;
        }

        DmaQueue_DirectCopy(width << 1, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (copyRow * 0x40)));
        row++;
    }

    dst = (u16*)&gEwramData->sharedScratchBuffer;
    copyRow = firstRow - 1;
    for (j = 0; j < width; j++)
    {
        *dst++ = 0xE2B0;
    }
    DmaQueue_DirectCopy(width << 1, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (copyRow * 0x40)));

    dst = (u16*)&gEwramData->sharedScratchBuffer;
    copyRow = firstRow + 3;
    for (j = 0; j < width; j++)
    {
        *dst++ = 0xE2B1;
    }
    DmaQueue_DirectCopy(width << 1, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (column * 2) + (copyRow * 0x40)));
}

// Matches (from decomp.me scratch iF3mv, 94.64 %: var_4/var_2 copies)
/**
 * @brief Show the soul name for (soul, kind) in the message window, with
 * text ids offset per soul kind; for a newly acquired soul also show
 * the extended window. Called by Pickup_Collect.
 */
void Hud_ShowSoulAcquiredMessage(s32 arg0, s32 arg1, s32 arg2)
{
    struct GameState *gameState;
    s32 bufferIndex;
    u8 *buffer;
    s32 first;
    s32 soulIndex;
    u8 *text;
    s32 textWidth;
    s32 row;
    s32 i;
    s32 column;
    u16 *dst;
    s32 width;
    s32 topX;
    s32 midY;
    s32 bottomY;
    s32 midX;
    s32 bottomX;
    s32 topY;

    gameState = &gEwramData->gameState;
    bufferIndex = gEwramData->gameState.hudTextBufferIndex;
    buffer = gEwramData->sharedScratchBuffer;
    first = 4;
    soulIndex = arg0;

    switch (arg1)
    {
        case 0:
            first = 1;
            arg0 += 0xE3;
            break;
        case 1:
            first = 2;
            arg0 += 0x11B;
            break;
        case 2:
            first = 3;
            arg0 += 0x133;
            break;
        case 3:
            first = 6;
            arg0 += 0x156;
            break;
    }

    if (gEwramData->gameState.stateFlags & STATE_HUD_MESSAGES_BLOCKED)
    {
        return;
    }
    if ((arg2 == 0) && (gEwramData->gameState.stateFlags & STATE_MESSAGE_WINDOW_CLOSED))
    {
        return;
    }

    Hud_CloseMessageWindow();
    text = Text_GetLocalizedEntry(arg0);
    Text_SetBackgroundColor(1);
    Text8_SetPixelCursor(0, 0);
    if (bufferIndex == 0)
    {
        Text8_SetRegion(0, 0, 0x14, 1);
        Text8_SetCursor(0, 0);
    }
    else
    {
        Text8_SetRegion(0, 1, 0x14, 1);
        Text8_SetCursor(0, 1);
    }
    Text8_Print((struct unk_08506B38 *) text, 0);
    textWidth = (Text8_GetPixelX() + 7) >> 3;
    Text8_Print((struct unk_08506B38 *) Text_GetLocalizedEntry(0x35E), 0);
    width = textWidth;

    if (arg2 != 0)
    {
        column = 0xF - ((width + 4) >> 1);
        
        if (Hud_IsWindowAreaFree() != 0)
        {
            row = 0xC;
        }
        else
        {
            row = 3;
        }
    }
    else
    {
        column = 0x1A - width;
        row = 1;
    }
    topX = column;
    topY = row;

    dst = (u16*)&gEwramData->sharedScratchBuffer[0];
    *dst++ = 0xE220;
    *dst++ = 0xE234;
    *dst++ = 0xE235;
    for (i = width - 2; i > 0; i--)
    {
        *dst++ = 0xE223;
    }
    *dst++ = 0xE622;
    *dst++ = 0xE621;
    *dst++ = 0xE620;
    DmaQueue_DirectCopy((width + 4) << 1, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (topX * 2) + (topY * 0x40)));
    midX = column;
    midY = row + 1;

    dst = (u16*)&gEwramData->sharedScratchBuffer[0];
    *dst++ = 0xE230;
    *dst++ = 0xE231;
    for (i = 0; i < width; i++)
    {
        *dst++ = (i + (bufferIndex << 5) + 0x80) | 0xE200;
    }
    *dst++ = 0xE631;
    *dst++ = 0xE630;
    DmaQueue_DirectCopy((width + 4) << 1, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (midX * 2) + (midY * 0x40)));
    bottomX = column;
    bottomY = row + 2;

    dst = (u16*)&gEwramData->sharedScratchBuffer[0];
    *dst++ = 0xE240;
    *dst++ = 0xE254;
    *dst++ = 0xE255;
    for (i = width - 2; i > 0; i--)
    {
        *dst++ = 0xE243;
    }
    *dst++ = 0xE642;
    *dst++ = 0xE641;
    *dst++ = 0xE640;
    DmaQueue_DirectCopy((width + 4) << 1, (u32 *)buffer, (u32 *) (VRAM_BASE + 0xE000 + (bottomX * 2) + ((bottomY) * 0x40)));

    BgPalette_LoadRange((u8 *)sSavePromptBgPalette, first, 1, 0xE);
    BgPalette_LoadRange((u8 *)sTextBgPalette, 1, 1, 0xF);
    if (arg2 != 0)
    {
        Hud_ShowLargeSoulWindow(arg1, soulIndex + 1);
        gameState->hudMessageTimer = 2;
    }
    else
    {
        gameState->hudMessageTimer = 0xF0;
    }

    gameState->hudMessageRequest = 0;
    gameState->hudTextBufferIndex = 1 - gameState->hudTextBufferIndex;
    gameState->hudMessageId = arg0;
}

/**
 * @brief Once per enemy (flag unk_3E bit 0), dismiss the area-name banner
 * and request the enemy name text from sEnemyNameTextIds[enemy id] unless
 * messages are blocked. Called by Hit_ResolveDamage.
 */
void Hud_RequestEnemyName(struct EwramData_EntityData *enemy)
{
    u16 hudMessageId;
    struct GameState *gameState;

    if (!(enemy->enemyFlags & 1))
    {
        AreaName_Dismiss(&gEwramData->gameState);
        hudMessageId = sEnemyNameTextIds[enemy->enemyId];
        gameState = &gEwramData->gameState;
        if (!(gameState->stateFlags & (STATE_HUD_MESSAGES_BLOCKED | STATE_LARGE_WINDOW_OPEN | STATE_MESSAGE_WINDOW_CLOSED)))
        {
            gameState->hudMessageRequest = 2;
            gameState->hudMessageId = hudMessageId;
        }
        enemy->enemyFlags |= 1;
    }
}

/**
 * @brief If the window is open, mark it closed (stateFlags bit 0x02000000),
 * clear its tilemap rows through the DMA queue and reset the request
 * and timer.
 */
void Hud_CloseMessageWindow(void)
{
    s32 vramAddr;
    s32 row;
    struct GameState *gameState;
    u8 *buffer;
    u32 size;
    u32 rows;
    gameState = &gEwramData->gameState;
    buffer = gEwramData->sharedScratchBuffer;

    if (!(gEwramData->gameState.stateFlags & STATE_MESSAGE_WINDOW_CLOSED))
    {
        gEwramData->gameState.stateFlags |= STATE_MESSAGE_WINDOW_CLOSED;
        gEwramData->hBlankEffect.requestStop = 1;

        size = 0x26;
        DMA_FILL_32(3, 0, buffer, size);
        vramAddr = 0x0600E01A;
        for (row = 0; row < 4; row++)
        {
            DmaQueue_DirectCopy(size, (u32 *)buffer, (u32 *) vramAddr);
            vramAddr += 0x40;
        }
        vramAddr = 0;

        row = 5;
        rows = 0x11;
        DMA_FILL_32(3, 0, buffer, rows * 64);
        DmaQueue_DirectCopy(rows * 64, (u32 *)buffer, (u32 *)(VRAM_BASE + 0xE000 + row * 0x40));
        gameState->hudMessageRequest = 0;
        gameState->hudMessageTimer = 0;
        gEwramData->gameState.stateFlags &= ~STATE_LARGE_WINDOW_OPEN;
    }
}

/**
 * @brief Enter the saved room (roomData) at the saved player and camera
 * positions and start its music, or load room data at (0, 0); clear
 * the H-blank buffer.
 */
void Room_LoadInitial(void)
{
    if (gEwramData->gameState.roomData != NULL)
    {
        gEwramData->gameState.currentRoomDefinition = (struct RoomDefinition *)gEwramData->gameState.roomData;
        gEwramData->playerState.player->posX.part16.integer = gEwramData->gameState.savedPlayerX;
        gEwramData->playerState.player->posY.part16.integer = gEwramData->gameState.savedPlayerY;

        Room_LoadAtCamera(gEwramData->gameState.savedCameraX, gEwramData->gameState.savedCameraY);
        if (Music_UpdateRoom(&gEwramData->gameState) == 0)
        {
            Music_PlayAreaQuiet();
            Music_SetCurrent(0);
            PlaySong(AUDIO_STOP);
        }
    }
    else
    {
        Room_LoadAtCamera(0U, 0U);
    }

    gEwramData->hBlankEffect.requestStop = 1;
    DMA_FILL_32(3, 0, &gEwramData->hBlankEffect.hBlankBuffer, sizeof(gEwramData->hBlankEffect.hBlankBuffer[0]));
}

/**
 * @brief Clamp the camera position to the current room size, load the room
 * layers there, apply its display/blend settings and refresh the map.
 */
void Room_LoadAtCamera(u16 cameraX, u16 cameraY)
{
    struct RoomDefinition *room;

    room = gEwramData->gameState.currentRoomDefinition;
    gEwramData->gameState.spawnCameraOffsetMode = 0;
    if (room != NULL)
    {
        if ((cameraX >> 8) >= room->bgLayers->dimensions->widthCells)
        {
            cameraX = (room->bgLayers->dimensions->widthCells << 0x8) + 0xFF00;
        }
        if ((cameraY >> 8) >= room->bgLayers->dimensions->heightCells)
        {
            cameraY = (room->bgLayers->dimensions->heightCells << 0x8) + 0xFF00;
        }
        Room_Load(room, cameraX, cameraY);
        gUnk_03002CB0.dispCnt = room->displayControl;
        gDisplayRegisters.bldCnt = room->blendControl;
        Display_SetDefaultBlend();
        Bg_UpdateScrollModes();
    }
    Hud_Init();
}

/**
 * @brief When gameState.cameraScrollMinPriority does not exceed priority, apply
 * the signed 16.16 deltas to BG1 with margins, scroll entities with it, refresh the map and
 * return 1; otherwise return 0.
 */
s32 Camera_ScrollWithPriority(s32 priority, s32 deltaX, s32 deltaY)
{
    if (gEwramData->gameState.cameraScrollMinPriority > priority)
        return 0;

    BgMap_ScrollWithMargin(1, deltaX, deltaY);
    Entity_ScrollWithBg(1);
    Bg_UpdateScrollModes();
    return 1;
}

/**
 * @brief Initial BG X scroll for a room layer from the BG1 X scroll and the
 * layer scroll mode: zero, doubled, halved or scaled by layer width.
 */
u16 BgLayer_GetInitialScrollX(u16 bg1ScrollX, u8 scrollMode, u8 widthCells, u16 pixelWidth)
{
    struct BgLayerState *bg;
    s32 bg1Width;
    u16 scrollX;

    scrollX = bg1ScrollX;
    bg = &gEwramData->bgInfo[1];

    switch (scrollMode)
    {
        case 0:
        case 8:
        case 9:
        case 10:
        case 14:
        case 17:
        case 18:
        case 20:
        case 21:
        case 22:
        case 27:
        case 28:
        case 29:
        case 30:
        case 32:
        case 33:
        case 35:
            scrollX = 0;
            break;

        case 5:
        case 7:
            scrollX *= 2;
            break;

        case 2:
        case 4:
            if (widthCells > 1)
            {
                scrollX /= 2;
            }
            else
            {
                scrollX = 0;
            }
            break;

        case 11:
        case 13:
        case 15:
        case 16:
        case 23:
        case 31:
            if (widthCells > 1)
            {
                bg1Width = bg->pixelWidth;
                scrollX = (scrollX * (pixelWidth - 0xF0)) / (bg1Width - 0xF0);
            }
            else
            {
                scrollX = 0;
            }
            break;

        case 36:
            break;
    }
    return scrollX;
}

/**
 * @brief Initial BG Y scroll for a room layer from the BG1 Y scroll and the
 * layer scroll mode, plus 0x30.
 */
u16 BgLayer_GetInitialScrollY(u16 bg1ScrollY, u8 scrollMode, u8 heightCells, u16 pixelHeight)
{
    struct BgLayerState *bg;
    s32 bg1Height;
    u16 scrollY;

    scrollY = bg1ScrollY;
    bg = &gEwramData->bgInfo[1];

    switch (scrollMode)
    {
        case 0:
        case 8:
        case 9:
        case 14:
        case 17:
        case 18:
        case 20:
        case 21:
        case 22:
        case 27:
        case 28:
        case 29:
        case 30:
        case 32:
        case 33:
        case 35:
            scrollY = 0;
            break;

        case 10:
            scrollY = 0xFFD0;
            break;

        case 6:
        case 7:
            scrollY *= 2;
            break;

        case 3:
        case 4:
            if (heightCells > 1U)
            {
                scrollY /= 2;
            }
            else
            {
                scrollY = 0;
            }
            break;

        case 12:
        case 13:
            if (heightCells <= 1U)
            {
                scrollY = 0;
            }
            else
            {
                bg1Height = bg->pixelHeight;
                scrollY = (scrollY * (pixelHeight - 0xD0)) / (bg1Height - 0xD0);
            }
            break;

        case 15:
        case 16:
            bg1Height = bg->pixelHeight;
            scrollY = (scrollY * (pixelHeight - 0xD0)) / (bg1Height - 0xD0);
            break;

        case 36:
            break;
    }

    return scrollY + 0x30;
}

/**
 * @brief Set text foreground color 15 and background color 1.
 */
void Text_SetDefaultColors(void)
{
    Text_SetForegroundColor(0xF);
    Text_SetBackgroundColor(1);
}

/**
 * @brief Clear the HUD font graphics at VRAM 0x06004000 (0x2000 bytes).
 */
void Hud_ClearGraphics(void)
{
    DMA_FILL_32(3, 0, VRAM_BASE + 0x4000, 0x2000);
}

/**
 * @brief Empty hook called at the end of Room_Load.
 */
void Room_OnLoaded(struct GameState *gameState)
{
    // TODO: is gameState unused or an implicit call?
    return;
}

/**
 * @brief Unless messages are blocked (stateFlags & (STATE_HUD_MESSAGES_BLOCKED | STATE_LARGE_WINDOW_OPEN | STATE_MESSAGE_WINDOW_CLOSED)), request pickup
 * message text id for the message window (Pickup_Collect).
 */
void Hud_RequestPickupMessage(s32 hudMessageId)
{
    struct GameState *gameState;

    gameState = &gEwramData->gameState;
    if (!(gameState->stateFlags & (STATE_HUD_MESSAGES_BLOCKED | STATE_LARGE_WINDOW_OPEN | STATE_MESSAGE_WINDOW_CLOSED)))
    {
        gameState->hudMessageRequest = 1;
        gameState->hudMessageId = hudMessageId;
    }
}

/**
 * @brief Process the pending message request hudMessageRequest: 1 shows a pickup
 * message, 2 an enemy name, 3 closes the window; otherwise count down
 * the display timer hudMessageTimer and close the window when it expires.
 */
void Hud_UpdateMessageWindow(struct GameState *gameState)
{
    switch (gameState->hudMessageRequest)
    {
        case 1:
            Hud_ShowPickupMessage(gameState->hudMessageId);
            return;

        case 2:
            Hud_ShowEnemyName(gameState->hudMessageId);
            return;

        case 3:
            Hud_CloseMessageWindow();
            return;

        default:
            if (gameState->hudMessageTimer != 0)
            {
                gameState->hudMessageTimer -= 1;
                if (gameState->hudMessageTimer == 0)
                {
                    Hud_CloseMessageWindow();
                }
            }
            return;
    }
}

/**
 * @brief Restore BG0 as the HUD text layer (font at VRAM 0x06004000, text
 * tiles at 0x06005000) and clear stateFlags bit 0x200.
 */
void Hud_EndEventTextLayer(void)
{
    DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(0, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    gDisplayRegisters.bgOfs[0].hOfs = 0;
    gDisplayRegisters.bgOfs[0].vOfs = 6;
    Text_SetTileBuffer(VRAM_BASE + 0x5000);
    Gfx_CopyResource((u8 *)sHudGfx, VRAM_BASE + 0x4000, 0);
    gEwramData->gameState.stateFlags &= ~STATE_HUD_MESSAGES_BLOCKED;
}

/**
 * @brief Switch BG0 to the event text layout (text tiles at 0x06004000),
 * clear its VRAM and set stateFlags bit 0x200, which blocks HUD messages.
 */
void Hud_BeginEventTextLayer(void)
{
    gDisplayRegisters.bgCnt[0] = CREATE_BGCNT(1, 28, BGCNT_HIGH_PRIORITY, BGCNT_SIZE_256x256);
    gEwramData->gameState.stateFlags |= STATE_HUD_MESSAGES_BLOCKED;
    Text_SetTileBuffer(VRAM_BASE + 0x4000);
    Text_SetBackgroundColor(0);
    DMA_FILL_32(3, 0, VRAM_BASE + 0x4000, 0x2000);
    DMA_FILL_32(3, 0, VRAM_BASE + 0x6000, 0x2000);
    DMA_FILL_32(3, 0, VRAM_BASE + 0xE000, 0x800);
}

/**
 * @brief Draw a HUD gauge row for value/max at the given tile position;
 * the last argument selects the HP (0) or MP (1) tile set.
 */
void Hud_DrawGauge(s32 x, s32 y, s32 value, s32 max, s32 style)
{
    // TODO: Hud_DrawGauge is the same as DrawStatGauge
    s32 i;
    u16 *dst;
    s32 fill;
    s32 baseTile;
    s32 palette;

    palette = 0xD;
    if (style == 0)
    {
        baseTile = 0x200;
    }
    else
    {
        baseTile = 0x210;
    }

    if (value > 0)
    {
        fill = Div(Div(value * 100, max) << 6, 100);
        if (fill == 0)
        {
            fill = 1;
        }
    }
    else
    {
        fill = 0;
    }

    dst = (u16*)(VRAM_BASE + 0xE000 + (x * 2) + (y << 6));
    for (i = 0; i < 8; i++)
    {
        if (i == fill / 8)
        {
            *dst = (baseTile + (fill - (i * 8))) | (palette << 12);
        }
        else
        {
            if (fill >= ((i + 1) * 8))
            {
                *dst = (baseTile + 8) | (palette << 12);
            }
            else
            {
                *dst = (baseTile) | (palette << 12);
            }
        }
        dst += 1;
    }
}

/**
 * @brief Unreferenced copy of the window placement test: 0 when an event
 * (stateFlags bit 0) runs with the player below y 0x70, else 1.
 */
s32 Hud_UseBottomWindow(void)
{
    // TODO: Hud_UseBottomWindow is the same as Hud_IsWindowAreaFree
    s32 res;
    res = 1;
    if ((gEwramData->gameState.stateFlags & STATE_EVENT_RUNNING) && (gEwramData->playerState.player->posY.part16.integer > 0x70))
        res = 0;
    return res;
}
