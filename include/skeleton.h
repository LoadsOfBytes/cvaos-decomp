#ifndef SKELETON_H
#define SKELETON_H

/* Functions of enemy/skeleton.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Skeleton_UpdatePatrol(struct EwramData_EntityData *entity);
extern void Skeleton_UpdateCombatMovement(struct EwramData_EntityData *entity);
extern void Skeleton_UpdateIdle(struct EwramData_EntityData *entity);
extern void Skeleton_UpdateAttackPreparation(struct EwramData_EntityData *entity);
extern void Skeleton_UpdateThrow(struct EwramData_EntityData *entity);
extern void Skeleton_UpdateDeath(struct EwramData_EntityData *entity);
extern void SkeletonLinkedPart_Update(struct EwramData_EntityData *entity);
extern void SkeletonAttachedPart_Update(struct EwramData_EntityData *entity);
extern void Skeleton_UpdateState5Animation(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateLinkedAndHide(struct EwramData_EntityData *entity, EntityFunc updateFunc);
extern void Skeleton_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

#endif /* SKELETON_H */
