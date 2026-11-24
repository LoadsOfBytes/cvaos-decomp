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
extern struct EwramData_EntityData *Effect_SpawnShared(s32 x, s32 y, s32 script, u32 palette);
extern struct EwramData_EntityData *Afterimage_Create(struct EwramData_EntityData *entity, u16 delay, u8 count, s32 flags);
extern void Effect_SpawnAttachedAnimation(struct EwramData_EntityData *parent, u8 anim, u8 palette, s32 offsetX, s32 offsetY);
extern void Effect_SpawnParticleTemplateList(s16 x, s16 y, s32 index);
extern struct EwramData_EntityData *Effect_CreatePlayerFlash(s32 kind);

#endif /* EFFECTS_H */
