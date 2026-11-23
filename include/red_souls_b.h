#ifndef RED_SOULS_B_H
#define RED_SOULS_B_H

/* Functions of soul/red_souls_b.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern u8 RedSoul_GetCastAnim(s32 pose);
extern u8 RedSoul_GetActiveLimit(void);
extern struct EwramData_EntityData *Player_GetSelectedOrNearestTarget(struct EwramData_EntityData *entity);
extern s32 Entity_IsOffscreenOrInactive(struct EwramData_EntityData *entity, s32 margin);
extern void Entity_SetAttackBoxFromFrame(struct EwramData_EntityData *entity);
extern void RedSoul_RemoveActiveEntity(struct EwramData_EntityData *soul);
extern void RedSoulEffect_UpdateAccelerated(struct EwramData_EntityData *entity);
extern void SoulBoneFragment_Update(struct EwramData_EntityData *entity);
extern void SoulAttack_MarkHit(struct EwramData_EntityData *entity);
extern void RedSoul_AnchorToHitTargetTimed(struct EwramData_EntityData *entity, struct EwramData_EntityData *target); // TODO: verify types
extern void Soul_DrawWithPaletteInvert(struct EwramData_EntityData *entity);
extern void Effect_UpdateBallisticDebris(struct EwramData_EntityData *entity);
extern void BoneSoul_OnPlayerContact(struct EwramData_EntityData *entity);
extern void SkillMermanDropletUpdate(struct EwramData_EntityData *entity);
extern void Entity_DrawWithVelocityCopies(struct EwramData_EntityData *entity);
extern void RedSoul_AnchorToHitTarget(struct EwramData_EntityData *entity, struct EwramData_EntityData *target);
extern void SkillCockatriceDraw(struct EwramData_EntityData *entity);

#endif /* RED_SOULS_B_H */
