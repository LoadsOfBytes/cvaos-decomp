#ifndef OBJECTS_15_18_H
#define OBJECTS_15_18_H

/* Functions of object/objects_15_18.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void ChaoticRealmPortalCreate(struct EwramData_EntityData *entity);
extern void ChaoticRealmPortalUpdate(struct EwramData_EntityData *entity);
extern void ChaoticRealmVortexUpdate(struct EwramData_EntityData *entity);
extern void ChaoticRealmVortexCreate(struct EwramData_EntityData *entity);
extern void ElevatorCreate(struct EwramData_EntityData *entity);
extern void ElevatorUpdate(struct EwramData_EntityData *entity);
extern void BreakableCoinContainerCreate(struct EwramData_EntityData *entity);
extern void BreakableCoinContainerUpdate(struct EwramData_EntityData *entity);
extern void SpinningBlade_OnPlayerContact(struct EwramData_EntityData *entity);

#endif /* OBJECTS_15_18_H */
