#ifndef OBJECTS_22_2A_H
#define OBJECTS_22_2A_H

/* Functions of object/objects_22_2a.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void InputChallengePlatformCreate(struct EwramData_EntityData *entity);
extern void InputChallengePlatformUpdate(struct EwramData_EntityData *entity);
extern void VerticalPlatformCreate(struct EwramData_EntityData *entity);
extern void VerticalPlatformUpdate(struct EwramData_EntityData *entity);
extern void HorizontalPlatformCreate(struct EwramData_EntityData *entity);
extern void HorizontalPlatformUpdate(struct EwramData_EntityData *entity);

#endif /* OBJECTS_22_2A_H */
