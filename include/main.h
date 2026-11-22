#ifndef MAIN_H
#define MAIN_H

#include "types.h"

void AgbMain(void);
void VblankInterrupt(void);
void ScreenShake_ApplyBgOffsets(void);
void ScreenShake_RemoveBgOffsets(void);
void EmptyInterrupt(void);
void VcountInterrupt(void);
void GameModeUpdate(void);
void SetGameMode(s32 gameMode);
void IncrementGameMode(void);
void Display_ResetRegisters(void);
void Game_ResetAll(void);
void InitializeGame(void);
void UpdateInput(void);
u32 SoftResetCheck(void);

#endif /* MAIN_H */
