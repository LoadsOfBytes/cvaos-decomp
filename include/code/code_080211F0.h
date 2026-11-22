#ifndef CODE_080211F0_H
#define CODE_080211F0_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Debug2229_FrameHook(void);
extern void Player_MarkAttackContact(void); // TODO: Unused param_0 and param_1, both struct EwramData_EntityData *
extern void Player_SetVelocityWithBraking(s32 velocityX, s32 velocityY);
extern void Player_SetKnockbackVelocity(s32 vx, s32 vy);
extern void Player_QueueEntity(struct EwramData_EntityData *entity, u8 collisionFlags);
extern s32 Player_GetFocusedActionFlags(void);
extern u8 Player_HasAbilityOneBased(s32 abilityIndexOneBased);
extern s32 Player_IsBusy(void);
extern void Player_EquipSoulsAndRestore(u8 redSoulId, u8 blueSoulId, u8 yellowSoulId);
extern void Player_SetXFraction(s32 fraction);
extern void Player_SetYFraction(s32 fraction);
extern void Player_RestoreHPAndMP(void);
extern s32 Player_IsBossTargetLowHp(void);
extern s32 Player_TryStartAnimation(struct EwramData_EntityData *entity, u8 animationIndex, u8 flags);
extern void Player_DrawWithOffset(struct EwramData_EntityData *entity);
extern void ButtonConfig_ResetDefaultsWrapper(void);
extern void Player_DrawMirrored(struct EwramData_EntityData *entity);
extern void Player_SetContactImmunity(s32 immunityValue);
extern u16 Player_CalculateAttackPower(u16 attackCategory);

struct PlayerContactDamage {
    u16 attack;
    u16 attributes;
    u16 unk_4;
    u16 reaction;
    u32 sourceX; /* 16.16 coordinate bits; original unsigned storage */
    u32 sourceY; /* 16.16 coordinate bits; original unsigned storage */
};
extern void Player_ApplyContactDamage(struct PlayerContactDamage *contactDamage);

extern s32 Player_TrackEntity(struct EwramData_EntityData *entity);
extern void Player_UpdateStatusAndRegen(struct EwramData_EntityData *entity);
extern void Player_RecalculateAttackDefense(void);
extern void PlayerStatusIcon_Update(struct EwramData_EntityData *entity);
extern s32 sub_08021DB0(void);
extern void Player_PruneTrackedEntities(void);
extern void Player_ClearStatusBit(s32 bitIndex);
extern void sub_08021E2C(void);
extern void sub_08021E30(void);
extern void sub_08021E34(void);
extern s32 Player_TestSoulStateFlags(s32 mask);
extern void Player_ClearStatusEffects(s32 mask);
extern void Player_SetStatusEffects(s32 mask, u16 timerValue);
extern void Player_CopyBaseStats(void);
extern void Player_AddPreparedStat(u16 statIndex, u16 amount);
extern s16 Player_CalculateAttack(s16 statValue);
extern s16 Player_CalculateDefense(s16 statValue);
extern void Player_ApplyPreparedStats(void);
extern void Player_RestoreHP(s32 amount);
extern void Player_RestoreMP(s32 amount);
extern void Player_ConsumeMP(s32 amount);
extern s32 Player_RegisterSolidObject(struct EwramData_EntityData *entity, void *callbackTable);
extern void Player_ClearSolidObject(void);
extern s32 Player_GetSolidObjectFlags(void);
extern s32 SolidObject_NoCorrection(void);
extern s32 SolidObject_NoCarry(void);
extern void Player_SpawnStatusIcon(s32 statusIndex);
extern void Player_StartContactImmunity(u8 frames);
extern void Player_ResetContactImmunity(void);
extern void Player_LockAndCancelAllSouls(void);
extern void Player_ClearStatusFlags(s32 mask);
extern void sub_080221C8(void);
extern void WeaponMelee_Update(struct EwramData_EntityData *entity);
extern void WeaponHeavy_Update(struct EwramData_EntityData *entity);
extern void Effect_UpdateGunCasing(struct EwramData_EntityData *entity);
extern void WeaponGun_Update(struct EwramData_EntityData *entity);
extern void WeaponValmanway_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData* Weapon_CreateAttackEntity(struct EwramData_EntityData *entity);
extern void JuliusWhipAttack_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData* Julius_CreateWhipAttackEntity(struct EwramData_EntityData *entity);

struct WeaponInfo {
    u16 name; // index into sItemDescriptionTextIds (EquipmentMenu_ShowSlotDescription)
    u8 pad_2[0x8 - 0x2];
    u8 attackAnimKind;
    u8 pad_9[0x10 - 0x9];
    u16 attackAttributes;
    u8 graphicsIndex;
    u8 spriteSheetIndex;
    u8 animScriptIndex;
    u8 paletteIndex;
    u8 attackVariant;
    u8 hitTimer;
    u16 attackSound;
    u8 pad_1A[0x1C - 0x1A];
};
extern struct WeaponInfo* Weapon_GetInfo(u8 weaponId);
extern struct WeaponInfo* Weapon_GetEquippedInfo(void);
extern struct WeaponInfo* Weapon_GetAttackerInfo(s32 slot);

extern u8 Weapon_GetPlayerAttackAnim(s32 attackType);
extern s32 Weapon_KeepsAttackOnLanding(void);
extern void WeaponAttack_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker); // TODO: param_0?
extern void WeaponGun_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker);
extern void WeaponGun_Draw(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData* RedSoul_CreateEntity(struct EwramData_EntityData *owner, u32 *update);
extern void RedSoul_CancelAll(void);
extern void RedSoulTrail_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData* RedSoul_CreateTrailCopy(struct EwramData_EntityData *entity, u8 delay);
extern s32 Entity_DropToFloor(struct EwramData_EntityData *entity);
extern void RedSoulAnchor_Update(struct EwramData_EntityData *entity);
extern void Skill_EvilButcher_Ripper_Use(struct EwramData_EntityData *entity);
extern void SkillBoomerangProjectileUpdate(struct EwramData_EntityData *entity);
extern void SkillAxeArmorTrailUpdate(struct EwramData_EntityData *entity);
extern void SkillAxeArmorUse(struct EwramData_EntityData *entity);
extern void HomingSoulProjectile_Update(struct EwramData_EntityData *entity);
extern void SkillChronomageUse(struct EwramData_EntityData *entity);
extern s32 Sprite_GetFirstBoxCenter(struct EwramData_EntityData *entity, u32 *outX, u32 *outY);
extern void SkillValkyrieUse(struct EwramData_EntityData *entity);
extern void SkillKillerDollUse(struct EwramData_EntityData *entity);
extern void SkillNemesisUse(struct EwramData_EntityData *entity);
extern u8 RedSoul_FindFreeEntitySlot(void);

struct RedSoulAttackView {
    u8 pad_0[0xA - 0x0];
    u16 baseAttack;
};
extern struct RedSoulAttackView *RedSoul_GetLastUsedInfo(void); // type?

struct RedSoulActivationView {
    void *updateFunc; // TODO: function signature
    u16 castAnimKind;
    s16 mpCost;
    u8 pad_8[0xE - 0x8];
    u16 param;
};
extern struct RedSoulActivationView* RedSoul_GetInfo(u8 soul);

extern u8 RedSoul_GetActiveLimit(void);
extern s32 Entity_IsOffscreenOrInactive(struct EwramData_EntityData *entity, s32 margin);
extern void Entity_SetAttackBoxFromFrame(struct EwramData_EntityData *entity);
extern s32 RandomSigned(s32 range);
extern void RedSoul_RemoveActiveEntity(struct EwramData_EntityData *soul);
extern void RedSoul_AnchorToHitTargetTimed(struct EwramData_EntityData *entity, struct EwramData_EntityData *target); // TODO: verify types
extern void HomingSoulProjectile_OnPlayerContact(struct EwramData_EntityData *param_0, struct EwramData_EntityData *param_1); // TODO: verify types
extern void Soul_DrawWithPaletteInvert(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *BlueSoul_CreateEntity(struct EwramData_EntityData *owner, void *update);
extern void BlueSoul_StopAll(void);
extern void Entity_PlaceRelativeTo(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, s32 offsetX, s32 offsetY); // TODO: param_2/param_3 should be u16
extern void YellowSoul_Stop(void);

#endif /* CODE_080211F0_H */
