#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

void MainMenu_SetHighlightWindow(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
void MainMenu_PrintMessage(s32 entry);
void MainMenu_DrawSlotList(struct EwramData_EntityData *entity);
void MainMenu_DrawCopySlotList(struct EwramData_EntityData *entity);
void MainMenu_DrawNameEntry(struct EwramData_EntityData *entity);
void SaveSelect_Draw(struct EwramData_EntityData *entity);
void MainMenu_DrawSelectionMarkers(struct EwramData_EntityData *entity, s16 x, s16 y);
void MainMenu_Draw(struct EwramData_EntityData *entity);
void MainMenu_InitEntity(struct EwramData_EntityData *entity);
void MainMenu_FreeEntity(struct EwramData_EntityData *entity);
void MainMenu_DrawSlotSummary(struct EwramData_EntityData *entity, struct SaveHeader *header, s32 tileX, s32 tileY, s32 panel);
void MainMenu_ShowSlotInfo(struct EwramData_EntityData *entity, s32 slot);
void MainMenu_ShowCopySlots(struct EwramData_EntityData *entity, s32 srcSlot, s32 dstSlot);
void MainMenu_InitGameStart(struct EwramData_EntityData *entity);
s32 MainMenu_UpdateGameStart(struct EwramData_EntityData *entity);
void MainMenu_InitDataCopy(struct EwramData_EntityData *entity);
s32 MainMenu_UpdateDataCopy(struct EwramData_EntityData *entity);
s32 MainMenu_UpdateDataDelete(struct EwramData_EntityData *entity);
void SaveSelect_ShowSlot(struct EwramData_EntityData *entity, s32 slot);
void SaveSelect_SetupMessageWindow(s32 choice);
void SaveSelect_InitScreen(struct EwramData_EntityData *entity);
s32 SaveSelect_Update(struct EwramData_EntityData *entity);
void MainMenu_InitDataDelete(struct EwramData_EntityData *entity);
void SaveSelect_Open(struct EwramData_EntityData *entity);
void SaveSelect_Close(struct EwramData_EntityData *entity);
void MainMenu_OpenNameEntry(struct EwramData_EntityData *entity);
s32 MainMenu_UpdateNameEntry(struct EwramData_EntityData *entity);
void GameFlags_SetAllExceptWord1(void);

#endif /* MAIN_MENU_H */
