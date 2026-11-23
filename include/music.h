#ifndef MUSIC_H
#define MUSIC_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 Music_UpdateRoom(struct GameState *gameState);
extern void Sound_ReplayTrackedWithVolume(void);
extern void Sound_StopTrackedWithVolume(void);
extern s32 Music_PlayAreaQuiet(void);
extern s32 Music_SetCurrent(s32 song);
extern void Sound_ClearTracked(void);
extern s32 Sound_PlayTracked(s32 song);
extern s32 Sound_PlayTrackedIfAbsent(s32 song);
extern s32 Sound_StopTracked(s32 song);
extern void Sound_ResumeAndReplayTracked(void);
extern void Sound_PauseAndStopTracked(void);
extern void Music_StartOverride(s32 song);
extern void Music_StopOverride(void);
extern void Music_SubmitCommandOnce(s32 song);
extern s32 GameModeGameOverUpdate(void);
extern void GameOverText_Draw(struct EwramData_EntityData *entity);
extern s32 GameModeCreditsUpdate(void);

#endif /* MUSIC_H */
