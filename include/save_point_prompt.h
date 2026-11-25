#ifndef SAVE_POINT_PROMPT_H
#define SAVE_POINT_PROMPT_H

/* Functions of object/save_point_prompt.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void SpinningBladeUpdate(struct EwramData_EntityData *entity);
extern void SpinningBlade_OnHit(struct EwramData_EntityData *entity);
extern void SpinningBlade_Draw(struct EwramData_EntityData *entity);
extern void ConveyorSwitchCreate(struct EwramData_EntityData *entity);
extern void ConveyorSwitchUpdate(struct EwramData_EntityData *entity);
extern void EmptyObject1ACreate(struct EwramData_EntityData *entity);
extern void EmptyObject1AUpdate(struct EwramData_EntityData *entity);
extern void SavePointCreate(struct EwramData_EntityData *entity);
extern void SavePointUpdate(struct EwramData_EntityData *entity);

#endif /* SAVE_POINT_PROMPT_H */
