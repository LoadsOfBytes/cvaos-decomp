#ifndef SCENE30_H
#define SCENE30_H

/* Functions of event/scene30.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 EventSceneEnding_PlayTransformation(struct EwramData_EntityData *entity);
extern void EventSceneEnding_SpawnFloatingFragment(struct EwramData_EntityData *owner, s16 x, s16 y, s32 alternate);
extern void EventSceneEndingEmitterA_Update(struct EwramData_EntityData *entity);
extern void EventSceneEndingEmitterB_Update(struct EwramData_EntityData *entity);
extern void EventSceneEndingEmitterC_Update(struct EwramData_EntityData *entity);
extern void EventSceneEndingEmitterD_Update(struct EwramData_EntityData *entity);
extern void EventSceneEndingEmitterE_Update(struct EwramData_EntityData *entity);
extern void EventSceneEndingPieceEmitter_Update(struct EwramData_EntityData *entity);
extern void EventSceneEnding_CreateCenterEffect(struct EwramData_EntityData *owner);
extern void EventSceneEnding_CreateHalo(struct EwramData_EntityData *owner);
extern void EventSceneEnding_StepPaletteTransition(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *EventSceneEnding_SpawnDriftFragment(s16 x,s16 y,s32 reverse,s32 alternate);

#endif /* SCENE30_H */
