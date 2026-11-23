#ifndef EFFECTS_H
#define EFFECTS_H

/* Functions of engine/effects.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern struct EwramData_EntityData* Effect_CreateFromTable(s32 x, s32 y, s32 ax, s32 ay, s32 index, u8 palette); // TODO: verify param types
extern s32 SoulAcquisition_CreateEffect(s32 kind);
extern struct EwramData_EntityData* Effect_CreateSharedFrame(s32 x, s32 y, s32 frame, u32 palette);
extern void Entity_EndIfActive(struct EwramData_EntityData *entity);
extern void PickupSource_Update(struct EwramData_EntityData* entity);
extern void PickupSource_Init(struct EwramData_EntityData* entity);
extern void PickupSource_Draw(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData* Effect_SpawnShared(s32 x, s32 y, s32 script, u32 palette);

#endif /* EFFECTS_H */
