#ifndef SCENE30_H
#define SCENE30_H

/* Functions of event/scene30.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 EventScene30_PlayTransformation(struct EwramData_EntityData *entity);
extern void EventScene30_SpawnFloatingFragment(struct EwramData_EntityData *owner, s16 x, s16 y, s32 alternate);
extern void EventScene30EmitterA_Update(struct EwramData_EntityData *entity);
extern void EventScene30EmitterB_Update(struct EwramData_EntityData *entity);
extern void EventScene30EmitterC_Update(struct EwramData_EntityData *entity);
extern void EventScene30EmitterD_Update(struct EwramData_EntityData *entity);
extern void EventScene30EmitterE_Update(struct EwramData_EntityData *entity);
extern void EventScene30PieceEmitter_Update(struct EwramData_EntityData *entity);
extern void EventScene30_CreateCenterEffect(struct EwramData_EntityData *owner);
extern void EventScene30_CreateHalo(struct EwramData_EntityData *owner);
extern void EventScene30_StepPaletteTransition(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *EventScene30_SpawnDriftFragment(s16 x,s16 y,s32 reverse,s32 alternate);

#endif /* SCENE30_H */
