#ifndef SOUND_H
#define SOUND_H

#include "types.h"

void StartSong(u16 song);
void AudioCommand(u16 command);
void MPlayVolumeSet(u16 player, u16 step, u16 volume);
void MPlayVolumeUpdate(void);
void InitializeAudio(void);
void AudioVSync(void);
extern void PlaySong(u16 song);
void ResetAudio(void);
u16 GetPlayerUpdatedVolume(u16 player);

#endif /* SOUND_H */
