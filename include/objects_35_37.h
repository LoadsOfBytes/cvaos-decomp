#ifndef OBJECTS_35_37_H
#define OBJECTS_35_37_H

/* Functions of object/objects_35_37.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void FloorSwitchGateCreate(struct EwramData_EntityData *entity);
extern void FloorSwitchGateUpdate(struct EwramData_EntityData *entity);
extern void RotatingPlatformUpdate(struct EwramData_EntityData *entity);
extern void RotatingPlatformCreate(struct EwramData_EntityData *entity);
extern void SwingDoorCreate(struct EwramData_EntityData *entity);
extern void SwingDoorUpdate(struct EwramData_EntityData *entity);
extern void Shop_SetupBackgrounds(void);
extern void Shop_Open(void);

#endif /* OBJECTS_35_37_H */
