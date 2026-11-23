#ifndef CODE_08011DD0_H
#define CODE_08011DD0_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 Map_CountVisitedCells(struct GameState *gameState);
extern s32 Map_GetCompletionTenths(struct GameState *gameState, s32 unused);
extern void Map_DrawCompletion(struct GameState *gameState, s32 unused, s32 column, s32 rowOffset);
extern void RoomJump_SetDestination(void *room, s32 cameraX, s32 cameraY, s32 playerX, s32 playerY);
extern void ScreenShake_RequestOffset(u8 priority, s8 offsetX, s8 offsetY);
extern void ScreenShake_Request(u8 priority, u8 duration, u8 maskX, u8 maskY);
extern void ScreenShake_Clear(void);
extern void GameFlag_Set(s32 flag);
extern s32 GameFlag_GetMask(s32 flag);
extern s32 GameFlag_TestAndSet(s32 flag);
extern s32 GameFlag_FindMapCoordinate(s32 x, s32 y);
extern s32 GameFlag_FindRoomCoordinate(s32 x, s32 y, s32 side);
extern void GameFlag_Clear(s32 flag);
extern void ItemIconCache_Upload(void);
extern void ItemIconCache_Init(void);
extern s32 ItemIconCache_Load(s32 id);
extern void ItemIconEntity_Update(struct EwramData_EntityData *entity);
extern void ItemIconCache_ClearIds(void);
extern s32 ItemIcon_AttachSprite(struct EwramData_EntityData *entity, u32 slot, s32 palette);
extern void ItemIcon_Free(struct EwramData_EntityData *entity, s32 release);
extern void ItemIconCache_ReleaseSlot(s32 slot);
extern s32 ItemIconCache_FindId(u32 id);
extern void ItemIconCache_Save(void);
extern void ItemIconCache_Restore(void);
extern void ItemIconCache_InvalidateUpload(void);
extern void SaveData_StorePlayerPosition(struct GameState *gameState);

#endif /* CODE_08011DD0_H */
