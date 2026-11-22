#ifndef CODE_08012744_H
#define CODE_08012744_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 SaveData_LoadSlotFromSram(s32 saveSlot);
extern s32 SaveData_ResaveSlotAfterEnding(s32 saveSlot, s32 fullClear);
extern s32 SaveData_DeleteSlot(s32 saveSlot);
extern s32 SaveData_CopySlot(s32 srcSaveSlot, s32 dstSaveSlot);
extern s32 SaveData_RemoveSoulInSlot(s32 saveSlot, s32 soulGroupOffset, s32 soulType, s32 soulIndex);
extern s32 SaveData_GiveAwaySoul(s32 saveSlot, s32 soulType, s32 soulId);
extern void Game_ResetProgressForNewGame(void);
extern s32 SaveData_CreateNewGameSlot(void);
extern void SaveData_UpdateSlotSummary(struct GameState *gameState);
extern u32 SaveData_SaveSlotToSram(s32 saveSlot);
extern s32 SaveData_ClearSlotInSram(s32 saveSlot);
extern s32 SaveData_ReadSlotHeader(s32 saveSlot, s32 bufferIndex);
extern u32 SaveData_SaveSystemFlags(void);
extern u32 SaveData_LoadSystemFlags(void);
extern s32 SaveSummary_GetLinkedSlot(struct SaveHeader *header);
extern s32 SaveData_RecordFullClear(void);
extern s32 SaveData_IsSlotUsed(s32 saveSlot, s32 headerIndex);
extern u32 SaveData_SaveLanguage(void);
extern s32 SaveData_LoadLanguage(void);

#endif /* CODE_08012744_H */
