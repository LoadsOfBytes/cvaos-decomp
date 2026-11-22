#ifndef CODE_0800F1FC_H
#define CODE_0800F1FC_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 RoomEntity_Spawn(struct RoomEntityRecord *record);
void RoomEntity_SpawnVisibleByX(struct RoomEntitySpawnState *spawnState, struct RoomEntityRecord *records);
void RoomEntity_SpawnVisibleByY(struct RoomEntitySpawnState *spawnState, struct RoomEntityRecord *records);
void RoomEntity_InitList(struct RoomEntityRecord *records);
void RoomEntity_SpawnOnScroll(void);
void RoomEntity_PreloadEnemies(struct RoomEntityRecord *records);
void Room_Load(struct RoomDefinition *room, u16 cameraX, u16 cameraY);
void Room_UnloadEntities(void);
s32 AreaName_GetPendingArea(struct GameState *gameState);
void AreaName_UpdateWave(s32 subModeCounter);
void AreaName_Show(struct GameState *gameState, s32 area);
s32 AreaName_Update(struct GameState *gameState);
void AreaName_Hide(struct GameState *gameState);
void RoomTransition_Begin(struct GameState *gameState);
void Room_LoadAtPlayerPosition(struct GameState *gameState);
s32 RoomTransition_Update(struct GameState *gameState);
s32 RoomFadeTransition_Update(struct GameState *gameState);
void Room_StoreEntryPosition(struct GameState *gameState);
void Room_LoadGraphics(struct RoomDefinition *room);
void Room_LoadPalettes(struct RoomDefinition *room);
void AreaName_Dismiss(struct GameState *gameState);
void RoomFadeTransition_Begin(struct GameState *gameState);
void Room_ClearForTransition(struct GameState *gameState);

#endif /* CODE_0800F1FC_H */
