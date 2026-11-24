#ifndef ENTITY_MOTION_H
#define ENTITY_MOTION_H

/* Functions of engine/entity_motion.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern s32 EntityDistance(struct EwramData_EntityData *entity, struct EwramData_EntityData *target);
extern u16 EntityAngleTo(struct EwramData_EntityData *entity, struct EwramData_EntityData *target);
extern s32 AngleTurnDirection(u16 from, u16 to);
extern s32 Entity_AccelerateAlongAngle(struct EwramData_EntityData *entity, u16 angle, s32 speed, s32 target, s16 acceleration, u16 time);
extern void SharedObjPalette_ResetSlots(void);
extern void Effect_UpdateParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateRandomlyAnimatedParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateTimedParticle(struct EwramData_EntityData *entity);
extern void Effect_UpdateParticleWithDamping(struct EwramData_EntityData *entity);
extern s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);
extern s32 Entity_AimVelocityAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);
extern s32 Entity_AimAccelerationAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);
extern s32 Entity_MoveTowardPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);
extern s32 Entity_MoveWithPointCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern s32 Entity_MoveWithCollisionAlt(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit);
extern s32 Entity_MoveWithClassifiedPointCollision(struct EwramData_EntityData *entity, s32 unused, s32 ceilingOffset, s32 checkCeiling);
extern s32 Entity_MoveAndCollide(struct EwramData_EntityData *entity, s32 sideOffset, s32 ceilingOffset, s32 checkCeiling, s32 speedLimit);
extern s32 Entity_MoveWithOptionalCeilingCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 checkCeiling);
extern s32 Entity_MoveAndProbePointCollision(struct EwramData_EntityData *entity, s32 unused, s32 ceilingOffset, s32 checkCeiling);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset, s32 speedLimit, s16 wallOffset);
extern u8 Angle_TurnTowards(u16 *angle, u16 step, u16 target);

#endif /* ENTITY_MOTION_H */
