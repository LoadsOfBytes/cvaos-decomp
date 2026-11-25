#ifndef ENTITY_COMMON_H
#define ENTITY_COMMON_H

/* Functions of engine/entity_common.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Effect_UpdateSpriteDebris(struct EwramData_EntityData *entity);
extern s32 Entity_MoveWithCollisionProfile(struct EwramData_EntityData *entity, s16 *profile, u16 stepSize, s32 flags);
extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags); // TODO: param_2 type
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern u32 RandomModuloAtLeast(u32 modulus, u32 minimum);
extern void GiantGhost_ResetAnimation(struct EwramData_EntityData *entity);
extern void Entity_DrawWithOffsetCopy(struct EwramData_EntityData *entity);
extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
extern s32 Target_GetFlipXOrOpposite(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);
extern s32 Entity_IsTargetBelow(struct EwramData_EntityData *entity);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern s32 Tile_Classify(s16 x, s16 y);
/* The same function for callers that were compiled without a prototype
 * (implicit int): the arguments are not narrowed. */
extern s32 Tile_ClassifyWide(s32 x, s32 y) asm("Tile_Classify");
extern s32 Collision_IsSlopeTile(s16 x, s16 y);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern s32 Entity_GetHorizontalRoomEdge(struct EwramData_EntityData *entity, s32 margin);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern s32 Entity_GetRoomPointDistance(struct EwramData_EntityData *entity, s32 x, s32 y);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *entity, s32 angle, s32 speed);
extern void EntityAccelerateX(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityMove(struct EwramData_EntityData *entity);
extern void EntityMoveX(struct EwramData_EntityData *entity);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityAccelerateAndMoveY(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *entity);
extern void Effect_SpawnAt(s32 x, s32 y, s32 layer);
extern void Effect_SpawnParticleAnim0(s32 x, s32 y, s32 vx, s32 vy, s32 layer);
extern void Effect_SpawnParticleAnim11(s32 x, s32 y, s32 vx, s32 vy, s32 layer);
extern void Effect_SpawnTimedFrameParticle(s32 x, s32 y, s32 vx, s32 vy, s32 layer, s32 damping, s32 flags, s32 palette, s32 duration);
extern void Effect_SpawnRisingParticle(s32 x, s32 y, s32 layer, s32 mode);
extern void Effect_SpawnFadingParticle(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant);
extern void Effect_SpawnConfiguredParticle(s32 x, s32 y, s32 velX, s32 velY, s32 spriteLayer, s32 variant, s32 flags, s32 palette);
extern void Effect_SpawnParticleWithDamping(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant, s32 damping, s32 flags, s32 palette);
extern void Effect_SpawnRisingParticleInBox(s32 x, s32 y, s32 width, s32 height, s32 layer, s32 mode);
extern void Effect_SpawnFadingParticleCopy(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 variant);
extern void Effect_SpawnParticleAnim12(s32 x, s32 y, s32 velX, s32 velY, s32 spriteLayer, s32 flags, s32 palette);
extern void Effect_SpawnStationaryAnimation(s32 x, s32 y, s32 layer);
extern void Effect_SpawnFallingPiece(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 piece);
extern void Effect_SpawnParticleAnim13(s32 x, s32 y, s32 velX, s32 velY, s32 layer, s32 paletteOffset);
extern s32 Entity_GetTargetOffsetX(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetOffsetY(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Entity_OnPlayerContactDamage(struct EwramData_EntityData *entity);
extern void Heap_Reset(void);
extern void Heap_SaveAndReset(void);
extern void Heap_Restore(void);
extern void Entity_SetVelocityXFromFacing(struct EwramData_EntityData *entity, s32 velocity);
extern s32 Entity_TryWalk(struct EwramData_EntityData *entity, s32 speed, s32 ceilingOffset, s32 velocityY, s32 skipSetup);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *entity);
extern u16 Attribute_GetHighestIndex(u16 attributes);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s32 Entity_DistanceToPoint(struct EwramData_EntityData *entity, s32 x, s32 y);
extern void Entity_HideAboveY(struct EwramData_EntityData *entity, s32 minY);
extern void RisingEffect_Update(struct EwramData_EntityData *entity);

extern void Sprite_GetPartCornerOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);
extern void Sprite_GetPartCenterOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);
extern void Sprite_GetPartOriginOffset(struct EwramData_EntityData *entity, s32 index, s16 *offset);
extern void EntityAccelerateY(struct EwramData_EntityData *entity, s32 maxSpeed);
extern void EntityMoveY(struct EwramData_EntityData *entity);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);
extern struct EwramData_EntityData *Entity_CreateObject(EntityFunc updateFunc);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *entity, u16 width, u16 height);
extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern s32 Target_GetPixelY(struct EwramData_EntityData *entity);
/* The same functions declared with their real s16 result: a sum with an s16
 * field is then done in 16 bits (callers in white_dragon.c and killer_fish.c). */
extern s16 Target_GetPixelXShort(struct EwramData_EntityData *entity) asm("Target_GetPixelX");
extern s16 Target_GetPixelYShort(struct EwramData_EntityData *entity) asm("Target_GetPixelY");
extern s32 Target_IsWithinDistance(struct EwramData_EntityData *entity, u32 maxDistance);
extern u32 Entity_StepTowardPoint(struct EwramData_EntityData *entity, s32 x, s32 y, u8 shift);
extern void HitEffect_SpawnAtContact(s32 attacker, struct EwramData_EntityData *entity, u16 type);
extern u32 Target_GetFlipX(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt);
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern s32 Target_IsInRect(s32 left_, s32 top_, s32 width_, s32 height_);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
/* The same function for callers that were compiled without a prototype
 * (implicit int): result and arguments are not narrowed. */
extern s32 Entity_LoadGraphicsWide(struct EwramData_EntityData *entity, u8 *resource, void *palette, s32 variant, void *sheet) asm("Entity_LoadGraphics");
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity, s32 offsetX_, s32 offsetY_);
extern void Entity_PlaceRelativeToCopy(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, s32 offsetX_, s32 offsetY_);
extern s32 Collision_FindWaterSurfaceOffset(s32 x__, s32 y__);

#endif /* ENTITY_COMMON_H */
