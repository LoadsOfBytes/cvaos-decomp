#ifndef CODE_08001194_H
#define CODE_08001194_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

void BgCmdBuffer_TransferToVram(void);
u16* BgCmdBuffer_WriteString(u16 tileIndex, u16 rowOffset, u8 tileInfo, u8 *string);
u16* BgCmdBuffer_WriteNumber(u16 tileIndex, u16 rowOffset, u8 palNum, s32 numChars, s32 value);
u16* BgCmdBuffer_WriteString_Duplicate(u16 tileIndex, u16 rowOffset, u8 tileInfo, u8 *string);
u16* BgCmdBuffer_WriteData(u8 bgCmd, u8 length, u8* src, u32 vramAddr);
void DmaQueue_Process(void);
struct DmaQueueInfo* DmaQueue_DirectCopy(u16 size, u32 *src, u32 *dst);
struct DmaQueueInfo* DmaQueue_IndirectCopy(u32 size, u32 *src, u32 *dst);
void HBlankEffectSetup(u8 numEntries, u8 vcountSetting, u8 writeSize, void *destReg);
s32 CheckRoomTransition(s32 xOffset, s32 yOffset);
s32 BgMap_GetBlockTileIndex(struct BgMapMetadata *metadata, u16 tileX, u16 tileY);
s32 GetAreaFromMapPosition(s32 xOffset, s32 yOffset);
s32 GetRoomFromMapPosition(s32 xOffset, s32 yOffset);
s32 GetSaveRoomFlagFromMapPosition(s32 xOffset, s32 yOffset);
s32 GetWarpRoomFlagFromMapPosition(s32 xOffset, s32 yOffset);
u32* GetRoomPointer(s32 area, s32 room);
u32 BgMap_GetTileEntry(struct BgMapMetadata *metadata, u16 tileX, u16 tileY);
u8 Collision_GetTile(s32 arg0, s32 arg1);
s16 Collision_GetSlopeHeight(u8 arg0, s16 arg1);
u8 Collision_FilterTile(u8 tile, s16 mode);
s16 Collision_GetPushDown(s32 x, s32 y);
s16 Collision_GetPushDownFiltered(s32 x, s32 y, s32 mode);
s16 Collision_GetPushUp(s32 x, s32 y);
s16 Collision_GetPushUpFiltered(s32 x, s32 y, s32 mode);
s32 Collision_IsPointSolid(s32 x, s32 y); // TODO: returns s32 or s16?
u8 Collision_GetTileAtEntity(struct EwramData_EntityData *entity, s32 dx, s32 dy);
u8 Collision_GetTileOnScreen(s32 x, s32 y);
s16 Collision_GetPushLeft(s32 x, s32 y);
s16 Collision_GetPushRight(s32 x, s32 y);
s16 Collision_GetGroundOffset(s32 x, s32 y);
s16 Collision_GetPushLeftFiltered(s32 x, s32 y, s32 mode);
s16 Collision_GetPushRightFiltered(s32 x, s32 y, s32 mode);
s16 Collision_GetGroundOffsetFiltered(s32 x, s32 y, s32 mode);
s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
u32 GetEntityRoomXPositionWhole(struct EwramData_EntityData *entity);
u32 GetEntityRoomYPositionWhole(struct EwramData_EntityData *entity);
void Collision_SetBlock(s32 arg0, s32 arg1);
void Collision_ClearBlock(s32 arg0, s32 arg1);
void Collision_ClearAllBlocks(void);
s16 Collision_TileEdgeRight(s32 x, s32 y, u8 tile);
s16 Collision_TileEdgeLeft(s32 x, s32 y, u8 tile);
void Window1_SetCircle(s16 centerX, s16 centerY, s16 radius);
void Window1_EnableHBlank(void);
void Window1_Disable(void);

#endif /* CODE_08001194_H */
