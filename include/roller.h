#ifndef ROLLER_H
#define ROLLER_H

/* Functions of object/roller.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void RollerCreate(struct EwramData_EntityData *entity);
extern void RollerUpdate(struct EwramData_EntityData *entity);

#endif /* ROLLER_H */
