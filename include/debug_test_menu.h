#ifndef DEBUG_TEST_MENU_H
#define DEBUG_TEST_MENU_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeDebugTestMenuUpdate(void);

// TODO: does this function actually use EntityData?
struct DebugEnemySpawnerState {
    u8 pad_0[0xA - 0x0];
    u8 state;
    u8 pad_B[0xD - 0xB];
    u8 spawnCounter;
    u8 pad_E[0x10 - 0xE];
    struct EwramData_EntityData *spawnedEntities[1]; // TODO: length?
};
void DebugTestMenu_UpdateTestEnemySpawner(struct DebugEnemySpawnerState *spawner);

s32 DebugTestMenu_StartGame(void);
s32 DebugTestMenu_HandleEnemySelectInput(void);
s32 DebugTestMenu_EnemyTest(void);
void DebugTestMenuObjTestViewer_Update(struct EwramData_EntityData *entity);
s32 DebugTestMenu_SoundTest(void);
s32 DebugTestMenu_ObjTest(void);
void DebugTestMenu_UnusedStub(void);
s32 DebugPlatform_GetCarryVelocity(struct EwramData_EntityData *entity);
void DebugTestEnemy_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
s32 DebugTestEnemy_UpdateHitFlash(struct EwramData_EntityData *entity);
void DebugTestEnemy_Update(struct EwramData_EntityData *entity);
void DebugTestEnemy_OnPlayerContact(struct EwramData_EntityData *entity);

extern const u8 sUnk_080E1F74[24];

#endif /* DEBUG_TEST_MENU_H */
