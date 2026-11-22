#ifndef CODE_08009A0_H
#define CODE_08009A0_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

u32 SetPlayerInput(s32 priority, s32 playerInputs);
s32 Sin(s32 angle);
u32 RandomNumberGenerator(void);
u32 RandomNumberGenerator2(void);
struct EwramData_EntityData* EntityCreate(u32 *updateFunc);
void Entity_UpdateAll(void);
void Entity_RemoveFromDrawList(struct EwramData_EntityData *entity);
void Entity_UpdateReserved(void);
void EntityDeleteAll(void);
struct EwramData_EntityData *EntityCreateInRange(s32 minSlot, s32 maxSlot, u32 *updateFunc);
void EntityDelete(struct EwramData_EntityData *entity);
void EntityFreeAll(void);
s32 EntityDeleteGroup(u8 group);
void EntityUpdateNothing(void);
u8 EntityGetSlot(struct EwramData_EntityData *entity);
void Entity_ScrollWithBg(s32 bg);
void Entity_AddToDrawList(struct EwramData_EntityData *entity);

#endif /* CODE_08009A0_H */
