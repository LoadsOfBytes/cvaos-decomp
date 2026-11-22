#ifndef CODE_0800B700_H
#define CODE_0800B700_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

void SoundTestMenu_DrawScreen(s32 index);
s32 GameModeSoundTestMenu(void);
s32 GameModeInGameUpdate(void);
s32 Game_SetMenusEnabled(s32 enabled);
s32 GameModeUnk3Update(void);
void InGame_InitScene(void);
void InGame_LoadCommonObjectPalettes(void);
void InGame_LoadCommonObjectGraphics(void);
void Display_SetDefaultBlend(void);
void Game_ResetSystems(struct GameState *gameState);
s32 InGame_UpdateDeath(void);
s32 InGame_UpdateTimerAndHud(void);
s32 InGame_EnterCredits(void);
void UpdateInGameTimer(void);

#endif /* CODE_0800B700_H */
