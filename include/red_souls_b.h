#ifndef RED_SOULS_B_H
#define RED_SOULS_B_H

/* Functions of soul/red_souls_b.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

/* Red soul / Julius sub-weapon entry (sSomaRedSoulInfo, sJuliusSubweaponInfo;
 * RedSoul_GetInfo): update function of the created entity, cast animation
 * kind, MP cost, limit of active entities, base attack power and a
 * parameter passed to the entity (RedSoulWork.param).
 */
struct RedSoulInfo {
    /* 0x00 */ void *updateFunc; // TODO: function signature
    /* 0x04 */ u16 castAnimKind;
    /* 0x06 */ s16 mpCost;
    /* 0x08 */ u8 activeEntityLimit;
    /* 0x09 */ u8 pad_9;
    /* 0x0A */ u16 baseAttack;
    /* 0x0C */ u8 pad_C[0xE - 0xC];
    /* 0x0E */ u16 param;
}; /* size = 0x10 */

extern const struct RedSoulInfo sSomaRedSoulInfo[]; /* Soma (currentCharacter == 0) */

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

extern const struct RedSoulInfo *RedSoul_GetLastUsedInfo(void);
extern const struct RedSoulInfo *RedSoul_GetInfo(u8 soul);

#endif /* RED_SOULS_B_H */
