#ifndef CODE_08035930_H
#define CODE_08035930_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebug1967Update(void);

// TODO: does this function actually use EntityData?
struct DebugEnemySpawnerState {
    u8 pad_0[0xA - 0x0];
    u8 state;
    u8 pad_B[0xD - 0xB];
    u8 spawnCounter;
    u8 pad_E[0x10 - 0xE];
    struct EwramData_EntityData *spawnedEntities[1]; // TODO: length?
};
void Debug1967_UpdateTestEnemySpawner(struct DebugEnemySpawnerState *spawner);

s32 Debug1967_StartGame(void);
s32 Debug1967_HandleEnemySelectInput(void);
s32 Debug1967_EnemyTest(void);
void Debug1967ObjTestViewer_Update(struct EwramData_EntityData *entity);
s32 Debug1967_SoundTest(void);
s32 Debug1967_ObjTest(void);
void sub_08036368(void);
s32 DebugPlatform_GetCarryVelocity(struct EwramData_EntityData *entity);
void DebugTestEnemy_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
s32 DebugTestEnemy_UpdateHitFlash(struct EwramData_EntityData *entity);
void DebugTestEnemy_Update(struct EwramData_EntityData *entity);
void DebugTestEnemy_OnPlayerContact(struct EwramData_EntityData *entity);

#endif /* CODE_08035930_H */
