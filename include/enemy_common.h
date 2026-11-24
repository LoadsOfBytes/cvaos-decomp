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
extern s32 Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *entity, s32 inset);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, u32 range);
extern u16 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *entity, u16 width, u16 height);
extern s32 Collision_IsWalkBlocked(s32 x_, s32 y_);
extern s32 Entity_MoveWithFilteredBoxCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 xOffset_, s32 yOffset_, s32 width_, s32 height_, s32 movement);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit, s16 inputDamage);
/* The same function for callers that were compiled without a prototype
 * (implicit int): result and arguments are not narrowed. */
extern s32 Hit_ResolveDamageWide(struct EwramData_EntityData *entity, struct EwramData_EntityData *other, u16 *hit, s32 inputDamage) asm("Hit_ResolveDamage");
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);

#endif /* ENEMY_COMMON_H */
