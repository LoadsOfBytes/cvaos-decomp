#ifndef BG_EFFECTS_H
#define BG_EFFECTS_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

void BgEffect_Bg3ScanlineParallax(void);
void BgEffect_DeepeningHWave(s32 bgNum, s32 top, s32 height, s32 depth);
void BgEffect_HWaveInRange(s32 bgNum, s32 top, s32 bottom, s32 amplitude);
void BgEffect_SkewWave(s32 bgNum, s32 top, s32 bottom);
void BgEffect_SkewWaveSteep(s32 bgNum, s32 top, s32 bottom);
void BgEffect_Bg2Perspective(void);
void BgEffect_Bg2RotateAndPulse(s32 scrX, s32 scrY, s32 rotateSpeed);
void Display_SaveState(void);
void Display_RestoreState(void);
void Bg_UpdateScrollModes(void);
void Hud_Init(void);
void Hud_DrawGaugeFrames(void);
void Hud_UpdateHpMpGauges(struct GameState *gameState);
void Hud_ShowEnemyName(s32 hudMessageId);
void Hud_ShowPickupMessage(s32 hudMessageId);
void Hud_ClearLargeWindowTiles(void);
void Hud_DrawWindowPrompt(void);
void Hud_ShowLargeSoulWindow(s32 soulType, s32 soulId);
void Hud_ShowSoulAcquiredMessage(s32 arg0, s32 arg1, s32 arg2);

struct EnemyHudNameView {
    u8 pad_0[0x36 - 0x0];
    u8 enemyId;
    u8 pad_37[0x3E - 0x37];
    u8 enemyFlags;
};
void Hud_RequestEnemyName(struct EnemyHudNameView *enemy);
void Hud_CloseMessageWindow(void);
void Room_LoadInitial(void);
void Room_LoadAtCamera(u16 cameraX, u16 cameraY);
s32 Camera_ScrollWithPriority(s32 priority, s32 deltaX, s32 deltaY);
u16 BgLayer_GetInitialScrollX(u16 bg1ScrollX, u8 scrollMode, u8 widthCells, u16 pixelWidth);
u16 BgLayer_GetInitialScrollY(u16 bg1ScrollY, u8 scrollMode, u8 heightCells, u16 pixelHeight);
void Text_SetDefaultColors(void);
void Hud_ClearGraphics(void);
void Room_OnLoaded(struct GameState *gameState);
void Hud_RequestPickupMessage(s32 hudMessageId);
void Hud_UpdateMessageWindow(struct GameState *gameState);
void Hud_EndEventTextLayer(void);
void Hud_BeginEventTextLayer(void);
void Hud_DrawGauge(s32 x, s32 y, s32 value, s32 max, s32 style);
s32 Hud_UseBottomWindow(void);

#endif /* BG_EFFECTS_H */
