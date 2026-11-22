#ifndef CODE_08008750_H
#define CODE_08008750_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

s32 GameModeBossRushMenuUpdate(void);
s32 GameModeUnk9Update(void);
s32 GameModeIntroCutsceneUpdate(void);
void Game_SetStartPosition(s32 arg0);
void IntroCutscene_UpdateRisingSprite(struct EwramData_EntityData *entity);
void IntroCutscene_DrawMirrored(struct EwramData_EntityData *entity);
void IntroCutscene_UpdateDriftingSprite(struct EwramData_EntityData *entity);
void IntroCutscene_UpdateShrinkingSprite(struct EwramData_EntityData *entity);
void IntroCutscene_UpdateFloatingSprite(struct EwramData_EntityData *entity);
s32 IntroCutsceneText_PrintNextLine(struct EwramData_EntityData *entity);
void IntroCutscene_CreateEntities(struct EwramData_EntityData *entity);
struct EwramData_EntityData* IntroCutscene_CreateSprite(u32 *updateFunc, u8 palette);
void IntroCutsceneText_Update(struct EwramData_EntityData *entity);

#endif /* CODE_08008750_H */
