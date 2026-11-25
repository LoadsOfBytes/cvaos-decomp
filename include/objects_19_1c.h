#ifndef OBJECTS_19_1C_H
#define OBJECTS_19_1C_H

/* Functions of object/objects_19_1c.c used by other files. */

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

#endif /* OBJECTS_19_1C_H */
