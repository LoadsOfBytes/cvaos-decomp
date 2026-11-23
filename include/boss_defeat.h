#ifndef BOSS_DEFEAT_H
#define BOSS_DEFEAT_H

/* Functions of enemy/boss_defeat.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void BossDefeatParticle_Update(struct EwramData_EntityData *entity);
extern void BossDefeatEffect_Spawn(s32 x, s32 y);
extern void BossDefeat_LoadEffectTiles(void);
extern void BossDefeat_RestoreEffectTiles(void);
extern void Headhunter_SpawnDefeatEffect(s32 x, s32 y);
extern void Chaos_SpawnDefeatEffect(s32 x, s32 y);

#endif /* BOSS_DEFEAT_H */
