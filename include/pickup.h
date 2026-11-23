#ifndef PICKUP_H
#define PICKUP_H

/* Functions of engine/pickup.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern struct EwramData_EntityData* Pickup_CreateItem(s32 x, s32 y, s32 type, s32 id, s32 hidden);
extern void Pickup_Collect(struct EwramData_EntityData *e);

#endif /* PICKUP_H */
