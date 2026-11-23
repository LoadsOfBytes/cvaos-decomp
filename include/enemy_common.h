#ifndef ENEMY_COMMON_H
#define ENEMY_COMMON_H

/* Functions of engine/enemy_common.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 Entity_AdjustSpeedForSlope(struct EwramData_EntityData *entity, s32 speed);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern s32 Entity_IsOutsideRoomMap(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void HitEffect_Update(struct EwramData_EntityData *entity);
extern s32 Entity_IsFacingTarget(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);
extern void Heap_Free(void *allocation);
extern void *Heap_Alloc(u16 words);
extern void Room_ResetTransientState(void);

#endif /* ENEMY_COMMON_H */
