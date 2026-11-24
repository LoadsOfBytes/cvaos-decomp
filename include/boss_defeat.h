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
extern void SoulPickup_SpawnFromEnemyDefinition(s32 x_, s32 y_, s32 enemyId, s32 spawnParam);
extern void BossRecoveryPickup_Spawn(u16 x, u16 y, s32 event, s32 delay, s32 special);
/* The same function for callers that were compiled without a prototype
 * (implicit int): the arguments are not narrowed. */
extern void BossRecoveryPickup_SpawnWide(s32 x, s32 y, s32 event, s32 delay, s32 special) asm("BossRecoveryPickup_Spawn");

#endif /* BOSS_DEFEAT_H */
