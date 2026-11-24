#ifndef SOUL_PICKUP_H
#define SOUL_PICKUP_H

/* Functions of engine/soul_pickup.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern struct EwramData_EntityData* SoulPickup_CreateContainer(s32 x, s32 y, s32 variant, s32 index);
extern void Pickup_UpdateBouncing(struct EwramData_EntityData *e);
extern const void *Item_GetInfo(s32 category, s32 index);
extern s32 Item_GetTypeByGlobalId(s32 globalId);
extern s32 Item_GetIndexByGlobalId(s32 globalId);
extern void Player_AddGold(s32 amount);
extern u8* ItemInventory_GetCountPtr(s32 type, s32 index);

extern void *Item_GetInfoByGlobalId(s32 id);
extern struct EwramData_EntityData *SoulPickup_Create(s32 x, s32 y, s32 palette, s32 soulId, s32 stationary);
extern struct EwramData_EntityData *Effect_CreateHomingOrb(struct EwramData_EntityData *owner, s32 x, s32 y, s32 dy, s32 palette);

#endif /* SOUL_PICKUP_H */
