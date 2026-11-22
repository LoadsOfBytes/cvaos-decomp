#ifndef CODE_08060B98_H
#define CODE_08060B98_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s8 ObjPalette_FindOrLoad(void *palette); // TODO: types?
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s32 left, s32 top, s32 width, s32 height);
extern void SharedObjPalette_ResetSlots(void);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Room_ResetTransientState(void);
extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags); // TODO: param_2 type
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityAccelerateAndMoveY(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor);
extern void Heap_Reset(void);
extern u16 Attribute_GetHighestIndex(u16 attributes);
extern void HitEffect_Spawn(s16 x, s16 y, u16 type);

#endif /* CODE_08060B98_H */
