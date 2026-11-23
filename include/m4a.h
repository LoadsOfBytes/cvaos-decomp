#ifndef GUARD_M4A_H
#define GUARD_M4A_H

#include "m4a_internal.h"

void m4aSoundVSync(void);

void m4aSoundInit(void);
void m4aSoundMain(void);
void m4aSongNumStart(u16);
void m4aSongNumStartOrChange(u16);
void m4aSongNumStartOrContinue(u16);
void m4aSongNumStop(u16 n);
void m4aMPlayAllStop(void);
void m4aMPlayAllContinue(void);
void m4aMPlayContinue(struct MP2KPlayerState *mplayInfo);
void m4aMPlayFadeOut(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayFadeOutTemporarily(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayFadeIn(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayImmInit(struct MP2KPlayerState *mplayInfo);

void m4aMPlayTempoControl(struct MP2KPlayerState *mplayInfo, u16 tempo);
void m4aMPlayVolumeControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, u16 volume);
void m4aMPlayPitchControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, s16 pitch);
void m4aMPlayPanpotControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, s8 pan);

extern u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust);
extern void MPlayContinue(struct MP2KPlayerState *mplayInfo);
extern void MPlayFadeOut(struct MP2KPlayerState *mplayInfo, u16 speed);
extern void MPlayExtender(struct MixerSource *cgbChans);
extern void ClearChain(struct MixerSource *chan);
extern void Clear64byte(void *addr);
extern void SoundInit(struct SoundMixerState *soundInfo);
extern void SampleFreqSet(u32 freq);
extern void m4aSoundMode(u32 mode);
extern void m4aSoundVSyncOff(void);
extern void m4aSoundVSyncOn(void);
extern void MPlayOpen(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *tracks, u8 trackCount);
extern void MPlayStart(struct MP2KPlayerState *mplayInfo, struct MP2KSongHeader *songHeader);
extern void MPlayStop(struct MP2KPlayerState *mplayInfo);
extern void FadeOutBody(struct MP2KPlayerState *mplayInfo);
extern void TrkVolPitSet(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track);
extern u32 MidiKeyToCgbFreq(u8, u8, u8);
extern void CgbOscOff(u8);
extern void CgbModVol(struct MixerSource *chan);
extern void CgbSound(void);
extern void ClearModM(struct MP2KTrack *track);
extern void m4aMPlayModDepthSet(struct MP2KPlayerState *mplayInfo, u16 trackBits, u8 modDepth);
extern void m4aMPlayLFOSpeedSet(struct MP2KPlayerState *mplayInfo, u16 trackBits, u8 lfoSpeed);
extern void MP2K_event_memacc(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xcmd(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xxx(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xwave(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xtype(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xatta(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xdeca(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xsust(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xrele(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xiecv(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xiecl(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xleng(struct MP2KPlayerState *, struct MP2KTrack *);
extern void MP2K_event_xswee(struct MP2KPlayerState *, struct MP2KTrack *);

#endif // GUARD_M4A_H
