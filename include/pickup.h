#ifndef PICKUP_H
#define PICKUP_H

/* Functions of engine/pickup.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern struct EwramData_EntityData* Pickup_CreateItem(s32 x, s32 y, s32 type, s32 id, s32 hidden);
extern void Pickup_Collect(struct EwramData_EntityData *e);
extern struct EwramData_EntityData *Pickup_CreateMana(s32 x, s32 y, s32 large, s32 jump);
extern struct EwramData_EntityData *Pickup_CreateGold(s32 x, s32 y, s32 index);
extern void ItemInventory_AddOneByGlobalId(s32 item);

#endif /* PICKUP_H */
