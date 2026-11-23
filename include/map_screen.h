#ifndef MAP_SCREEN_H
#define MAP_SCREEN_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void MapScreen_DrawCellTopEdge(s32 x, s32 y, s32 color, s32 border, s32 edge);
extern void MapScreen_DrawCellLeftEdge(s32 x, s32 y, s32 color, s32 border, s32 edge);
extern void MapScreen_DrawCell(struct GameState *gameState, s32 x, s32 y, s32 color);
extern void MapScreen_DrawMap(struct GameState *gameState);
extern void MapScreen_UpdatePlayerMarker(struct GameState *gameState);
extern s32 MapScreen_Update(struct GameState *gameState);
extern void MapScreen_Open(struct GameState *gameState);
extern void Map_MarkCurrentCell(struct GameState *gameState);
extern void WarpMenu_HandleCursorInput(void);
extern void WarpMenu_Open(struct GameState *gameState);
extern s32 WarpMenu_Update(struct GameState *gameState);
extern s32 Room_IsPlayerInside(struct EwramData_EntityData *entity);
extern void RoomJump_Begin(struct GameState *gameState);
extern s32 RoomJump_Update(struct GameState *gameState);
extern void ScreenShake_Update(void);

#endif /* MAP_SCREEN_H */
