#ifndef EWRAM_STRUCTS_H
#define EWRAM_STRUCTS_H

#include "types.h"

struct InputData {
    /* 0x00014 */ u16 heldInput;
    /* 0x00016 */ u16 newInput;
    /* 0x00018 */ u16 repeatedInput;
    /* 0x0001A */ u16 repeatedInputTimer:10;
    /* 0x0001B */ u8 playerInputPriority:6;
    /* 0x0001C */ u16 playerHeldInput;
    /* 0x0001E */ u16 playerNewInput;
}; /* size = 0xC */

struct HBlankEffect {
    /* 0x07864 */ u8 requestStart:1;
    /* 0x07864 */ u8 requestStop:1;
    /* 0x07864 */ u8 enabled:1;
    /* 0x07864 */ u16 currentBuffer:1;
    /* 0x07865 */ u8 vcountSetting;
    /* 0x07866 */ u8 writeSize;
    /* 0x07867 */ u8 pad_7867[0x07868 - 0x07867];
    /* 0x07868 */ void *destReg;
    /* 0x0786C */ s16 hBlankBuffer[2][0x500];
}; /* size = 0x1408 */

/* Entity work slots 0x4F4..0x518 are reused by different entity routines.
 * whole, part16, part8 and bytes describe storage views, not fixed-point
 * units or gameplay roles. Keep the offset-based containers until their
 * per-entity meanings are established. In particular, unk_4F8_entity may
 * hold an owner, a child, or a cast heap-data pointer; it is not a universal
 * parent link. Preserve the declared types, signedness and array shapes.
 */
struct EwramData_unk4F8_8 {
    s8 unk_4F8;
    u8 unk_4F9;
    u8 unk_4FA;
    u8 unk_4FB;
};

struct EwramData_unk4F8_16 {
    u16 unk_4F8;
    s16 unk_4FA;
}; 

union EwramData_unk4F8 {
    struct EwramData_EntityData *unk_4F8_entity;
    s32 whole;
    struct EwramData_unk4F8_16 part16;
    struct EwramData_unk4F8_8 part8;
    s8 bytes[4];
};

struct EwramData_unk4FC_8 {
    s8 unk_4FC;
    u8 unk_4FD;
    u8 unk_4FE;
    u8 unk_4FF;
};

struct EwramData_unk4FC_16 {
    s16 unk_4FC;
    s16 unk_4FE[1]; // TODO: what is happening here?
}; 

union EwramData_unk4FC {
    struct EwramData_EntityData *unk_4FC_entity; // child effect (EventScene26_Init)
    u32 whole; // TODO: Player_Create stores a pointer here
    struct EwramData_unk4FC_16 part16;
    struct EwramData_unk4FC_8 part8;
    u8 bytes[4];
};

struct EwramData_unk500_8 {
    u8 unk_500;
    s8 unk_501;
    u8 unk_502;
    u8 unk_503;
};

struct EwramData_unk500_16 {
    s16 unk_500;
    s16 unk_502;
}; 

// Devil body part, 8 of them are pointed to by unk_500 (0x080914C4..0x08091FB7;
// shared by Flame Demon, Arc Demon and Demon Lord). Only used fields named.
struct DevilPart {
    /* 0x00 */ s32 offsetX; // x offset, 16.16 (negated when turning around)
    /* 0x04 */ s32 offsetY; // y offset, 16.16
    /* 0x08 */ s32 rotationAngle; // rotation angle (0x20000 - rotationAngle when turning around)
    /* 0x0C */ u8 pad_C[0x10 - 0xC];
    /* 0x10 */ s32 targetSwingAngle; // interpolation target for swingAngle
    /* 0x14 */ s32 swingAngle; // current hanging angle, interpolated toward targetSwingAngle
    /* 0x18 */ s32 anchorXOrSentinel; // absolute anchor X; 0x80000000 selects an indexed part
    /* 0x1C */ s32 anchorYOrPartIndex; // part index in sentinel mode, otherwise absolute anchor Y
    /* 0x20 */ s32 savedOffsetX; // saved offsetX (Devil_SaveParts / Devil_RestoreParts)
    /* 0x24 */ s32 savedOffsetY; // saved offsetY
    /* 0x28 */ s32 linkLength; // supplied or measured attachment length; native scale retained
}; /* size = 0x2C */

// 0x20-byte body part pointed to by unk_500 (enemy code around
// 0x080809C0..0x08081128, 11 parts, see Golem_UpdateParts). Only used fields named.
struct GolemPart {
    /* 0x00 */ s32 offsetX; // x offset, 16.16
    /* 0x04 */ s32 offsetY; // y offset, 16.16
    /* 0x08 */ s32 rotationAngle; // mirrored as 0x20000 - value when turning (Golem_FaceTarget)
    /* 0x0C */ s32 targetSwingAngle; // interpolation target for swingAngle
    /* 0x10 */ s32 swingAngle; // current hanging angle, interpolated toward targetSwingAngle
    /* 0x14 */ s32 anchorXOrSentinel; // absolute anchor X; 0x80000000 selects an indexed part
    /* 0x18 */ s32 anchorYOrPartIndex; // part index in sentinel mode, otherwise absolute anchor Y
    /* 0x1C */ s32 linkLength; // supplied or measured attachment length; native scale retained
}; /* size = 0x20 */

// Great Armor body part, 14 of them are pointed to by unk_500
// (0x0808DAFC..0x0808EA0F, GreatArmorSlashEffect_Update). Only used fields named.
struct GreatArmorPart {
    /* 0x00 */ s32 offsetX; // x offset, 16.16
    /* 0x04 */ s32 offsetY; // y offset, 16.16
    /* 0x08 */ s32 rotationAngle; // rotation angle (0x20000 - rotationAngle when turning around, GreatArmor_FaceTarget)
    /* 0x0C */ s32 targetSwingAngle; // interpolation target for swingAngle
    /* 0x10 */ s32 swingAngle; // current hanging angle, interpolated toward targetSwingAngle
    /* 0x14 */ s32 anchorXOrSentinel; // absolute anchor X; 0x80000000 selects an indexed part
    /* 0x18 */ s32 anchorYOrPartIndex; // part index in sentinel mode, otherwise absolute anchor Y
    /* 0x1C */ s32 savedOffsetX; // saved offsetX (GreatArmor_SaveParts / GreatArmor_RestoreParts)
    /* 0x20 */ s32 savedOffsetY; // saved offsetY
    /* 0x24 */ s32 linkLength; // supplied or measured attachment length; native scale retained
}; /* size = 0x28 */

// 0x30-byte body part pointed to by unk_500: Stolas has 6 of them
// (0x0809B478..0x0809C41B), Creaking Skull 13 (0x08097B94..0x08098C83);
// both use machine-identical helper code.
struct EntityJointPart {
    /* 0x00 */ s32 offsetX; // x offset, 16.16 (negated when turning around)
    /* 0x04 */ s32 offsetY; // y offset, 16.16
    /* 0x08 */ s32 rotationAngle; // rotation angle (0x20000 - rotationAngle when turning around)
    /* 0x0C */ u8 pad_C[0x10 - 0xC];
    /* 0x10 */ s32 targetSwingAngle; // interpolation target for swingAngle
    /* 0x14 */ s32 swingAngle; // current hanging angle, interpolated toward targetSwingAngle
    /* 0x18 */ s32 anchorXOrSentinel; // absolute anchor X; 0x80000000 selects an indexed part
    /* 0x1C */ s32 anchorYOrPartIndex; // part index in sentinel mode, otherwise absolute anchor Y
    /* 0x20 */ s32 savedOffsetX; // saved offsetX (Stolas_SaveJointPositions / Stolas_RestoreJointPositions)
    /* 0x24 */ s32 savedOffsetY; // saved offsetY
    /* 0x28 */ s32 linkLength; // supplied or measured attachment length; native scale retained
    /* 0x2C */ u8 pad_2C[0x30 - 0x2C];
}; /* size = 0x30 */

// Big Golem body part (joint), 14 of them are pointed to by unk_500
// (0x080AC014..0x080AD00B). Only the fields used so far are named.
struct BigGolemPart {
    /* 0x00 */ s32 offsetX; // x offset, 16.16
    /* 0x04 */ s32 offsetY; // y offset, 16.16
    /* 0x08 */ s32 rotationAngle; // rotation angle (0x20000 - rotationAngle when turning around, BigGolem_FaceTarget)
    /* 0x0C */ s32 targetSwingAngle; // interpolation target for swingAngle
    /* 0x10 */ s32 swingAngle; // current hanging angle, interpolated toward targetSwingAngle
    /* 0x14 */ s32 anchorXOrSentinel; // absolute anchor X; 0x80000000 selects an indexed part
    /* 0x18 */ s32 anchorYOrPartIndex; // part index in sentinel mode, otherwise absolute anchor Y
    /* 0x1C */ s32 savedOffsetX; // saved offsetX (BigGolem_SaveParts/BigGolem_RestoreParts)
    /* 0x20 */ s32 savedOffsetY; // saved offsetY
    /* 0x24 */ s32 linkLength; // supplied or measured attachment length; native scale retained
    /* 0x28 */ s32 interpolationTimer; // current interpolation divisor/countdown; update uses 1 when it is zero
    /* 0x2C */ s32 interpolationDuration; // duration used by the easing phases; can increase during slowdown
    /* 0x30 */ s32 interpolationDurationLimit; // eased attachment limit for the growing duration
    /* 0x34 */ s32 easingState; // 0 normal, 1 wait, 2 increase duration, 3 limit reached
}; /* size = 0x38 */

union EwramData_unk500 {
    s32 whole;
    struct BigGolemPart *bigGolemParts; // 14 parts, see struct BigGolemPart
    struct DevilPart *devilParts; // 8 parts, see struct DevilPart
    struct GreatArmorPart *greatArmorParts; // 14 parts, see struct GreatArmorPart
    struct EntityJointPart *jointParts; // Stolas (6), Creaking Skull (13)
    struct GolemPart *golemParts; // see struct GolemPart
    struct EwramData_EntityData *unk_500_entity; // parent of object 0x25 spawns
    struct EwramData_unk500_16 part16;
    struct EwramData_unk500_8 part8;
};

struct EntityPosX16 {
    u16 fraction;
    u16 integer;
}; 

union EntityPosX {
    s32 whole;
    struct EntityPosX16 part16;
};

struct EntityPosY16 {
    u16 fraction;
    s16 integer;
}; 

union EntityPosY {
    s32 whole;
    struct EntityPosY16 part16;
};

struct EwramData_unk504_8 {
    u8 unk_504;
    u8 unk_505;
    s8 unk_506;
    u8 unk_507;
};

struct EwramData_unk504_16 {
    s16 unk_504;
    s16 unk_506[1];
}; 

union EwramData_unk504 {
    struct EwramData_EntityData *unk_504_entity;
    s32 whole;
    struct EwramData_unk504_16 part16;
    struct EwramData_unk504_8 part8;
};

struct EntityVelY8 {
    s8 fraction0;
    u8 fraction1;
    u8 integer0;
    u8 integer1;
};

struct EntityVelY16 {
    s16 fraction;
    s16 integer;
}; 

union EntityVelY {
    s32 whole;
    struct EntityVelY16 part16;
    struct EntityVelY8 part8;
};

struct EntityVelX8 {
    s8 fraction0;
    u8 fraction1;
    u8 integer0;
    u8 integer1;
};

struct EntityVelX16 {
    s16 fraction;
    s16 integer;
}; 

union EntityVelX {
    s32 whole;
    struct EntityVelX16 part16;
    struct EntityVelX8 part8;
};

struct EwramData_unk508_8 {
    u8 unk_508;
    u8 unk_509;
    u8 unk_50A;
    u8 unk_50B;
};

struct EwramData_unk508_16 {
    s16 unk_508;
    s16 unk_50A;
}; 

union EwramData_unk508 {
    struct EwramData_EntityData *unk_508_entity; // EventScene03TargetOrb_OnContact effect target
    s32 whole;
    struct EwramData_unk508_16 part16;
    struct EwramData_unk508_8 part8;
};

struct EwramData_unk514_8 {
    u8 unk_514;
    u8 unk_515;
    u8 unk_516;
    u8 unk_517;
};

struct EwramData_unk514_16 {
    s16 spawnParam0; /* RoomEntityRecord.spawnParam0 (Room entity spawner); code-spawned entities reuse it (variant, event, type) */
    s16 spawnParam1; /* RoomEntityRecord.spawnParam1; also reused per entity type (index, count, item id) */
}; 

union EwramData_unk514 {
    struct EwramData_unk514_8 part8;
    struct EwramData_unk514_16 part16;
    struct EwramData_EntityData *unk_514_32;
};

struct EwramData_unk4F4_8 {
    u8 unk_4F4;
    u8 unk_4F5;
    u8 unk_4F6;
    u8 unk_4F7;
};

struct EwramData_unk4F4_16 {
    s16 unk_4F4;
    s16 unk_4F6;
}; 

struct WhipChainNode {
    s32 posX;
    s32 posY;
    s32 velX;
    s32 velY;
};

union EwramData_unk4F4 {
    struct EwramData_EntityData *unk_4F4_entity;
    s32 *unk_4F4_p32; // TODO: investigate
    struct WhipChainNode *whipNodes; // TODO: investigate
    s32 whole;
    struct EwramData_unk4F4_16 part16;
    struct EwramData_unk4F4_8 part8;
};

/*
 * Entity index convention: EntityCreate / EntityCreateInRange store the slot
 * index of a new entity (gEwramData->entityData[index]) in unk_4F4.part8.unk_4F4;
 * child entities of many families keep their parent's index in
 * unk_4F4.part8.unk_4F5 and look the parent up with
 * gEwramData->entityData[ENTITY_PARENT_INDEX(e)]. Some entity types reuse these
 * bytes for other data, so the macros are only used where the code follows the
 * convention. Token-identical expansion.
 */
#define ENTITY_INDEX(e) e->ext.raw.unk_4F4.part8.unk_4F4
#define ENTITY_PARENT_INDEX(e) e->ext.raw.unk_4F4.part8.unk_4F5

/*
 * Both index bytes read as one halfword: stored into a byte slot it gives the
 * entity index; Entity_CreateLinkedAndHide copies it whole into the linked
 * part, which then checks that its owner slot still holds the same entity.
 */
#define ENTITY_INDEX_PAIR(e) e->ext.raw.unk_4F4.part16.unk_4F4

/*
 * Owner pointer convention: many entity types created by another entity keep
 * their owner / parent in unk_4F8.unk_4F8_entity (weapons, souls, enemy parts,
 * effects). Used where the code reads it as the owner or stores the creator.
 */
#define ENTITY_OWNER(e) e->ext.raw.unk_4F8.unk_4F8_entity

/*
 * Creator pointer convention: projectiles, attack effects and parts spawned
 * by many enemies keep their creator in unk_4F4.unk_4F4_entity (family headers
 * name the same member e.g. ext.chaosChild.link, ext.legionPart.owner). Used where the
 * code stores the creator or reads it back as the parent.
 */
#define ENTITY_CREATOR(e) e->ext.raw.unk_4F4.unk_4F4_entity

struct EwramData_unk50C_8 {
    u8 unk_50C;
    u8 unk_50D;
    u8 unk_50E;
    u8 unk_50F;
};

struct EwramData_unk50C_16 {
    s16 unk_50C;
    s16 unk_50E;
}; 

union EwramData_unk50C {
    s32 whole;
    struct EwramData_unk50C_16 part16;
    struct EwramData_unk50C_8 part8;
};

struct EwramData_unk510_8 {
    u8 basePalette; /* palette slot restored after flashes (entity->palette = basePalette), copied to child parts */
    u8 hitFlashTimer; /* enemies: set to 8 on damage, Enemy_UpdateHitFlash alternates palette 4 / basePalette while it counts down */
    u8 unk_512;
    u8 unk_513;
};

struct EwramData_unk510_16 {
    s16 unk_510;
    s16 unk_512;
}; 

union EwramData_unk510 {
    s32 whole;
    struct EwramData_unk510_8 part8;
    struct EwramData_unk510_16 part16;
};

struct EntityAccelX8 {
    u8 fraction0;
    u8 fraction1;
    u8 integer0;
    u8 integer1;
};

struct EntityAccelX16 {
    s16 fraction;
    s16 integer;
}; 

union EntityAccelX {
    s32 whole;
    struct EntityAccelX8 part8;
    struct EntityAccelX16 part16;
};

struct EntityAccelY8 {
    u8 fraction0;
    u8 fraction1;
    u8 integer0;
    u8 integer1;
};

struct EntityAccelY16 {
    s16 fraction;
    s16 integer;
}; 

union EntityAccelY {
    s32 whole;
    struct EntityAccelY8 part8;
    struct EntityAccelY16 part16;
};

struct EwramData_unk518_8 {
    u8 unk_518;
    u8 unk_519;
    u8 enemyId; /* index into sEnemyData, copied from the owner to its parts */
    u8 spawnIndex; /* RoomEntityRecord.spawnIndex, copied to parts/projectiles; some types reuse it (tile, event) */
};

struct EwramData_unk518_16 {
    u16 hp; /* enemy HP (sEnemyData[enemyId].hp); other entity types reuse it (e.g. an angle) */
    u16 unk_51A;
}; 

union EwramData_unk518 {
    s32 whole;
    struct EwramData_unk518_8 part8;
    struct EwramData_unk518_16 part16;
};

/*
 * Hit system (Hit_*, HitBox_*, code_08041474.c .. code_080426B0.c).
 *
 * Every entity that takes part in collisions has an attack box and a body
 * box in gEntityBoxes[hit.boxSlot] (set per animation step, see AnimScript
 * types 2/3, or by HitBox_Set*). Hit_Check(a, b) hits b when a's attack box
 * (or, without noContactDamage, a's body box) overlaps b's body box. An
 * attack type already listed in the target's hitBy[] cannot hit again until
 * its hitTimer runs out (sHitTimers / hitTimerOverride), which gives the
 * invulnerability frames. The overlap rectangle of the last check is
 * gEwramData->hitRect*. Callbacks: onHit(self, other, result) on the
 * attacker, onHurt(self, other, result) on the entity that was hit.
 * entity_hit.h has the same block as struct HitInfo with typed callbacks.
 */
struct EntityHit {
    /* 0x00554 */ u8 type; /* attack type, indexes sHitTimers */
    /* 0x00555 */ u8 noContactDamage; /* nonzero: only the attack box hurts */
    /* 0x00556 */ u8 flags; /* bit 0: active, bit 1: cannot hit, bit 2: cannot be hit */
    /* 0x00557 */ u8 boxSlot; /* EntityGetSlot() index into gEntityBoxes */
    /* 0x00558 */ u8 hitBy[3]; /* attack types that hit recently */
    /* 0x0055B */ u8 pad_55B;
    /* 0x0055C */ u8 hitTimer[3]; /* frames until hitBy[i] is cleared */
    /* 0x0055F */ u8 pad_55F;
    /* 0x00560 */ void *onHit; /* HitCallback, see entity_hit.h */
    /* 0x00564 */ void *onHurt; /* HitCallback */
};

/*
 * Animation scripts (Anim_*, code_0803E820.c). A script is a list of steps;
 * each step shows one SpriteFrame (entity->animFrame) for 'duration' frames.
 * The step layout depends on AnimScript.type (sAnimStepFuncs):
 *   0 - steps points to separate AnimStep records ({frame, duration}),
 *   1 - 4-byte steps stored inline from &steps ({frame, duration, -, -}),
 *   2 - 8-byte inline steps {u16 frame, u16 duration, u32 hitbox}
 *       (the packed box becomes the entity's attack box, HitBox_SetAttackRaw),
 *   3 - 12-byte inline steps with a second packed box for the body box (HitBox_SetBodyRaw).
 * entity->animFlags: 1 = loop, 2 = paused, 4 = finished, 8 = the script was
 * given directly (animScript) instead of by index into the sheet.
 * Anim_Step* return 1 = same step, 2 = next step, 3 = finished, 4 = looped;
 * entity->animDone is set at the end of the script.
 */
struct AnimStep {
    u8 frame;
    u8 duration;
    u16 wideDuration;
    u32 attackBox;
    u32 bodyBox;
    u8 pad_C[0x10 - 0xC];
}; /* size = 0x10 */

struct AnimScript {
    u16 stepCount;
    u16 type; /* step layout, see above */
    struct AnimStep *steps; /* type 0; types 1-3 store the steps from here on */
};

/*
 * Per-entity-type work area of an entity (0x4F4..0x50F, ext). The raw view
 * keeps the offset-named slot unions; per-type structures are added to the
 * union family by family (variant 2) and replace the slot accessor macros.
 * agbcc (GCC 2.95) does not support anonymous unions, so every access names
 * the view: e->ext.raw.unk_4FC.part8.unk_4FD.
 */
struct EntityWorkRaw {
    /* 0x004F4 */ union EwramData_unk4F4 unk_4F4;
    /* 0x004F8 */ union EwramData_unk4F8 unk_4F8;
    /* 0x004FC */ union EwramData_unk4FC unk_4FC;
    /* 0x00500 */ union EwramData_unk500 unk_500;
    /* 0x00504 */ union EwramData_unk504 unk_504;
    /* 0x00508 */ union EwramData_unk508 unk_508;
    /* 0x0050C */ union EwramData_unk50C unk_50C;
}; /* size = 0x1C */

#include "structs/entity_work.h"

union EntityWork {
    struct EntityWorkRaw raw;
    struct GiantGhostWork giantGhost;
    struct GiantGhostHandWork giantGhostHand;
    struct GiantGhostAuraWork giantGhostAura;
    struct DeadCrusaderWork deadCrusader;
    struct ImpWork imp;
    struct PeepingEyeWork peepingEye;
    struct TsuchinokoWork tsuchinoko;
    struct WerewolfWork werewolf;
    struct DeadWarriorWork deadWarrior;
    struct DebugWork debug;
    struct FleamanWork fleaman;
    struct MudmanShotWork mudmanShot;
    struct PersephoneWork persephone;
    struct PushBlockWork pushBlock;
    struct NeedlesWork needles;
    struct NeedlesDeathWork needlesDeath;
    struct DoorWork door;
    struct KickerSkeletonWork kickerSkeleton;
    struct MimicWork mimic;
    struct MimicTriggerWork mimicTrigger;
    struct SkyFishWork skyFish;
    struct YellowSoulWork yellowSoul;
    struct GhostSpawnerWork ghostSpawner;
    struct GhostWork ghost;
    struct MandragoraWork mandragora;
    struct MandragoraDeathWork mandragoraDeath;
    struct BgParticleWork bgParticle;
    struct EctoplasmWork ectoplasm;
    struct Event09Work event09;
    struct HeadStackWork headStack;
    struct SkullArcherWork skullArcher;
    struct SkullArcherArrowWork skullArcherArrow;
    struct SkullMillioneWork skullMillione;
    struct SkullMillionePartWork skullMillionePart;
    struct HarpyWork harpy;
    struct HarpyFeatherWork harpyFeather;
    struct LilithWork lilith;
    struct LilithLungeWork lilithLunge;
    struct RipperWork ripper;
    struct RipperProjectileWork ripperProjectile;
    struct WaiterSkeletonWork waiterSkeleton;
    struct WaiterTrayWork waiterTray;
    struct GhostDancerWork ghostDancer;
    struct MedusaHeadWork medusaHead;
    struct QuezlcoatlWork quezlcoatl;
    struct QuezlcoatlSegmentWork quezlcoatlSegment;
    struct EvilButcherProjectileWork evilButcherProjectile;
    struct EvilButcherDebrisWork evilButcherDebris;
    struct GateWork gate;
    struct GateDrawWork gateDraw;
    struct KillerMantleWork killerMantle;
    struct KillerMantlePartWork killerMantlePart;
    struct GolemWork golem;
    struct GolemSegmentWork golemSegment;
    struct GolemPreviewWork golemPreview;
    struct ArachneWork arachne;
    struct KyomaDemonWork kyomaDemon;
    struct NightmareWork nightmare;
    struct NightmareEffectWork nightmareEffect;
    struct WheelWork wheel;
    struct WheelDeathWork wheelDeath;
    struct BaelWork bael;
    struct JointWork joint;
    struct JointPartWork jointPart;
    struct JointAttackWork jointAttack;
    struct BlueSoulWork blueSoul;
    struct BossPickupWork bossPickup;
    struct BossPickupParticleWork bossPickupParticle;
    struct LayerObjectWork layerObject;
    struct MinotaurWork minotaur;
    struct MinotaurAxeWork minotaurAxe;
    struct MinotaurTrailWork minotaurTrail;
    struct StolasWork stolas;
    struct StolasEffectWork stolasEffect;
    struct WormWork worm;
    struct RedSoulWork redSoul;
    struct RedSoulAnchorWork redSoulAnchor;
    struct DollWork doll;
    struct DollPartWork dollPart;
    struct KillerDollWork killerDoll;
    struct LightningDollWork lightningDoll;
    struct BelmontWork belmont;
    struct BelmontWeaponWork belmontWeapon;
    struct BelmontSceneWork belmontScene;
    struct BelmontParticleWork belmontParticle;
    struct KillerFishWork killerFish;
    struct KillerFishEffectWork killerFishEffect;
    struct KillerFishSwarmWork killerFishSwarm;
    struct MenuPageWork menuPage;
    struct MenuPageListWork menuPageList;
    struct MenuWork menu;
    struct MenuTitleWork menuTitle;
    struct UkobackWork ukoback;
    struct UkobackFireballWork ukobackFireball;
    struct UkobackSparkleWork ukobackSparkle;
    struct UkobackPieceWork ukobackPiece;
    struct AlastorWork alastor;
    struct AlastorPartWork alastorPart;
    struct AlastorPatternWork alastorPattern;
    struct SuccubusWork succubus;
    struct WerejaguarWork werejaguar;
    struct WerejaguarEffectWork werejaguarEffect;
    struct BigGolemPartWork bigGolemPart;
    struct BigGolemWork bigGolem;
    struct BigGolemAttackWork bigGolemAttack;
    struct CreakingSkullWork creakingSkull;
    struct CreakingSkullPartWork creakingSkullPart;
    struct CreakingSkullGuideWork creakingSkullGuide;
    struct CreakingSkullProjectileWork creakingSkullProjectile;
    struct CockatriceWork cockatrice;
    struct CockatricePartWork cockatricePart;
    struct CockatriceBeamWork cockatriceBeam;
    struct Event03Work event03;
    struct Event03OrbWork event03Orb;
    struct Event03PartWork event03Part;
    struct SlimeWork slime;
    struct NemesisWork nemesis;
    struct GrahamWork graham;
    struct GrahamPartWork grahamPart;
    struct GrahamSceneWork grahamScene;
    struct GrahamTurnWork grahamTurn;
    struct GrahamEmitterWork grahamEmitter;
    struct GladiatorWork gladiator;
    struct GladiatorSpinnerWork gladiatorSpinner;
    struct GladiatorPartWork gladiatorPart;
    struct GladiatorArmWork gladiatorArm;
    struct GladiatorShotWork gladiatorShot;
    struct AltairWork altair;
    struct AltairDropWork altairDrop;
    struct AltairFeatherWork altairFeather;
    struct DevilWork devil;
    struct DevilPartsWork devilParts;
    struct DevilPartWork devilPart;
    struct DevilProjectileWork devilProjectile;
    struct DevilChargeWork devilCharge;
    struct SkeletonWork skeleton;
    struct SkeletonDeathWork skeletonDeath;
    struct SkeletonPieceWork skeletonPiece;
    struct SkeletonBoneWork skeletonBone;
    struct WitchWork witch;
    struct WitchSpellWork witchSpell;
    struct WitchOrbitWork witchOrbit;
    struct WitchGroundWork witchGround;
    struct WitchCastWork witchCast;
    struct DiscArmorWork discArmor;
    struct DiscArmorHandWork discArmorHand;
    struct DiscArmorDiscWork discArmorDisc;
    struct DiscArmorRingWork discArmorRing;
    struct DiscArmorSparkWork discArmorSpark;
    struct ManEaterWork manEater;
    struct ManEaterBodyWork manEaterBody;
    struct ManEaterLinkWork manEaterLink;
    struct PickupWork pickup;
    struct AluraUneWork aluraUne;
    struct AluraUneThornWork aluraUneThorn;
    struct AluraUneRootWaveWork aluraUneRootWave;
    struct AfterimageWork afterimage;
    struct UneSpawnerWork uneSpawner;
    struct UneWork une;
    struct WingedSkeletonSpawnerWork wingedSkeletonSpawner;
    struct WingedSkeletonWork wingedSkeleton;
    struct ZombieWork zombie;
    struct ZombieSpawnerWork zombieSpawner;
    struct ZombiePieceWork zombiePiece;
    struct ZombieSpriteWork zombieSprite;
    struct TritonWork triton;
    struct TritonSpearWork tritonSpear;
    struct TritonShotWork tritonShot;
    struct TritonWaveWork tritonWave;
    struct MermanWork merman;
    struct MermanSpawnerWork mermanSpawner;
    struct GorgonWork gorgon;
    struct GreatArmorWork greatArmor;
    struct GreatArmorPartWork greatArmorPart;
    struct GreatArmorSlashWork greatArmorSlash;
    struct ManticoreWork manticore;
    struct ManticoreTailWork manticoreTail;
    struct BeamSkeletonWork beamSkeleton;
    struct BeamSkeletonBeamWork beamSkeletonBeam;
    struct BeamSkeletonPieceWork beamSkeletonPiece;
    struct DeathWork death;
    struct DeathHelperWork deathHelper;
    struct DeathWeaponWork deathWeapon;
    struct DeathFloatingWeaponWork deathFloatingWeapon;
    struct DeathBladeWork deathBlade;
    struct DeathPillarWork deathPillar;
    struct GrahamShotWork grahamShot;
    struct GrahamSpiralWork grahamSpiral;
    struct GrahamSpreadWork grahamSpread;
    struct WhiteDragonWork whiteDragon;
    struct WhiteDragonAimWork whiteDragonAim;
    struct WhiteDragonSegmentWork whiteDragonSegment;
    struct WhiteDragonChargeWork whiteDragonCharge;
    struct Event23Work event23;
    struct Event24Work event24;
    struct Event26Work event26;
    struct Event27Work event27;
    struct Event28Work event28;
    struct Event31Work event31;
    struct ParticleWork particle;
    struct ParticleTimedWork particleTimed;
    struct ParticlePairWork particlePair;
    struct ParticleTableWork particleTable;
    struct ParticleAttachWork particleAttach;
    struct PlayerEffectSetWork playerEffectSet;
    struct SurfaceEffectWork surfaceEffect;
    struct HitEffectWork hitEffect;
    struct PlayerFlashWork playerFlash;
    struct FragmentSetWork fragmentSet;
    struct SoulAcquireWork soulAcquire;
    struct LegionWork legion;
    struct LegionPartWork legionPart;
    struct LegionCorpseWork legionCorpse;
    struct LegionShellWork legionShell;
    struct HeadhunterWork headhunter;
    struct HeadhunterEffectWork headhunterEffect;
    struct HeadhunterOrbitWork headhunterOrbit;
    struct HeadhunterWhipWork headhunterWhip;
    struct HeadhunterWhipSegmentWork headhunterWhipSegment;
    struct HeadhunterChildWork headhunterChild;
    struct HeadhunterPartWork headhunterPart;
    struct Event30Work event30;
    struct Event30EffectWork event30Effect;
    struct Event30RingWork event30Ring;
    struct Event30FragmentWork event30Fragment;
    struct Event30DriverWork event30Driver;
    struct BaloreWork balore;
    struct BalorePartWork balorePart;
    struct BaloreHitBoxWork baloreHitBox;
    struct BaloreHandWork baloreHand;
    struct BaloreEffectWork baloreEffect;
    struct BalorePieceWork balorePiece;
    struct BaloreOverlayWork baloreOverlay;
    struct BaloreEmitterWork baloreEmitter;
    struct SavePointWork savePoint;
    struct SavePointPromptWork savePointPrompt;
    struct SavePointPartWork savePointPart;
    struct SavePointLightWork savePointLight;
    struct ChaosWork chaos;
    struct Chaos2Work chaos2;
    struct ChaosChildWork chaosChild;
    struct Chaos2PartWork chaos2Part;
    struct ChaosOrbWork chaosOrb;
    struct ChaosShotWork chaosShot;
    struct ChaosSoulShotWork chaosSoulShot;
    struct ChaosSnakeWork chaosSnake;
    struct ChaosRingWork chaosRing;
    struct ChaosDebrisWork chaosDebris;
    struct PlayerWork player;
    struct SolidObjectWork solidObject;
    struct SkillHomingWork skillHoming;
    struct SkillDeathSickleWork skillDeathSickle;
    struct SkillAlastorWork skillAlastor;
    struct SkillRedMinotaurWork skillRedMinotaur;
    struct SkillWingedSkeletonWork skillWingedSkeleton;
    struct SkillGladiatorWork skillGladiator;
    struct SkillChronomageWork skillChronomage;
    struct SkillThrowWork skillThrow;
    struct SkillWitchWork skillWitch;
    struct SkillBoomerangWork skillBoomerang;
    struct SkillUkobackWork skillUkoback;
    struct SkillDiscArmorWork skillDiscArmor;
    struct SkillCockatriceWork skillCockatrice;
    struct SkillBlueCrowWork skillBlueCrow;
    struct SkillSlimeWork skillSlime;
    struct SkillCreakingSkullWork skillCreakingSkull;
    struct SkillBonePillarWork skillBonePillar;
    struct SkillBoneWork skillBone;
    struct SkillRipperWork skillRipper;
    struct SkillNemesisWork skillNemesis;
    struct SkillSirenWork skillSiren;
    struct SkillWalkerWork skillWalker;
    struct SkillNightmareWork skillNightmare;
    struct SkillKyomaDemonWork skillKyomaDemon;
    struct SkillKyomaDemonWaitWork skillKyomaDemonWait;
    struct SkillGrowScaleWork skillGrowScale;
    struct SkillCatoblepasWork skillCatoblepas;
    struct Object0DWork object0D;
    struct Object16Work object16;
    struct Object37Work object37;
    struct Object37ChildWork object37Child;
    struct Object14Work object14;
    struct Object14SwingWork object14Swing;
    struct Object31Work object31;
    struct Object35Work object35;
    struct Object35ChildWork object35Child;
    struct Object36Work object36;
    struct Object30LeafWork object30Leaf;
    struct Object19Work object19;
    struct Object15Work object15;
    struct Object08Work object08;
    struct Object08DustWork object08Dust;
    struct Object1BWork object1B;
    struct Object25SpawnerWork object25Spawner;
    struct Object25Work object25;
    struct Object17Work object17;
    struct WarpRoomWork warpRoom;
    struct Object26Work object26;
    struct WarpGateWork warpGate;
    struct Object1DWork object1D;
    struct Object22Work object22;
    struct Object22PartWork object22Part;
    struct Object07Work object07;
    struct Object09Work object09;
    struct Object10Work object10;
    struct BossRushTimerWork bossRushTimer;
    struct Object1FWork object1F;
    struct TitleMenuWork titleMenu;
    struct MainMenuWork mainMenu;
    struct TitleLogoWork titleLogo;
    struct TitleEffectsWork titleEffects;
    struct TitleParticleWork titleParticle;
    struct TitleSpiralWork titleSpiral;
    struct TitleFlashWork titleFlash;
    struct IntroSpriteWork introSprite;
    struct IntroDriftWork introDrift;
    struct IntroTextWork introText;
    struct SoulTradeWork soulTrade;
    struct SoundTestTextWork soundTestText;
    struct ItemIconWork itemIcon;
    struct StatusIconWork statusIcon;
    struct WeaponWork weapon;
    struct WeaponGunWork weaponGun;
    struct WhipChainWork whipChain;
    struct WhipSegmentWork whipSegment;
    struct SubweaponWork subweapon;
    struct SubweaponUnusedWork subweaponUnused;
    struct GrandCrossWork grandCross;
    struct Object34Work object34;
    struct Object34PartWork object34Part;
    struct Event04Work event04;
    struct AnimViewerWork animViewer;
    struct SpriteDebrisWork spriteDebris;
    struct ValkyrieWork valkyrie;
    struct SkillNeedlesWork skillNeedles;
    struct SkillManEaterWork skillManEater;
    struct SkillCreakingSkullSegmentWork skillCreakingSkullSegment;
    struct DebugWalkerWork debugWalker;
    struct NumberIndicatorWork numberIndicator;
    struct SoulPickupWork soulPickup;
    struct SoulContainerWork soulContainer;
    struct HomingOrbWork homingOrb;
    struct SoulSelectorWork soulSelector;
    struct BgPaletteAnimWork bgPaletteAnim;
    struct WarpRoomSideLightWork warpRoomSideLight;
    struct WarpRoomUnusedPartWork warpRoomUnusedPart;
    struct PeepingEyeBodyWork peepingEyeBody;
    struct PeepingEyeLinkWork peepingEyeLink;
    struct LilithShotWork lilithShot;
    struct BatWork bat;
    struct DryadSeedWork dryadSeed;
    struct AxeArmorAxeWork axeArmorAxe;
    struct BomberArmorBombWork bomberArmorBomb;
    struct RockArmorRockWork rockArmorRock;
    struct ChronomageCastWork chronomageCast;
    struct ChronomageWork chronomage;
    struct StudentWitchWork studentWitch;
    struct DeathRewardWork deathReward;
    struct DeathDebrisWork deathDebris;
    struct TinyDevilWork tinyDevil;
    struct BlueCrowWork blueCrow;
    struct RedCrowWork redCrow;
    struct Chaos2EffectWork chaos2Effect;
};

/*
 * Entity (gEwramData->entityData[0xE0], code_080009A0.c Entity*).
 * Slots 0x00-0xCF are updated by Entity_UpdateAll (not while paused), the
 * reserved slots 0xD0-0xDF by Entity_UpdateReserved. An entity is drawn by
 * its drawFunc when it is in the draw list (drawNext/drawPrev, head
 * gEwramData->drawListHead; 0xFF = end, 0xFE = not linked). state, subState,
 * stage and timer are the generic state machine variables of the update
 * functions; the unions unk_4F4..unk_518 are per-entity-type variables.
 */
struct EwramData_EntityData {
    /* 0x004E4 */ u32 *updateFunc; // TODO: function signature
    /* 0x004E8 */ u32 *drawFunc; // TODO: function signature
    /* 0x004EC */ u8 drawNext; /* next entity in the draw list */
    /* 0x004ED */ u8 drawPrev;
    /* 0x004EE */ u8 state; /* main state of the update function */
    /* 0x004EF */ u8 subState;
    /* 0x004F0 */ u8 stage;
    /* 0x004F1 */ u8 timer; /* generic frame counter */
    /* 0x004F2 */ u8 drawSkipCounter; /* see drawSkip */
    /* 0x004F3 */ u8 drawSkip; /* nonzero: Sprite_Draw draws every (drawSkip + 1)-th frame */
    /* 0x004F4 */ union EntityWork ext; /* per-entity-type work area 0x4F4..0x50F */
    /* 0x00510 */ union EwramData_unk510 unk_510;
    /* 0x00514 */ union EwramData_unk514 unk_514;
    /* 0x00518 */ union EwramData_unk518 unk_518;
    /* 0x0051C */ s32 unk_51C; // affine angle accumulator (EventScene30Effect_UpdateSpinning / EventScene30Effect_UpdateSpinningKeepSlot)
    /* 0x00520 */ u8 attack; /* contact attack strength, passed to Player_ApplyContactDamage */
    /* 0x00521 */ u8 defense; // active damage-mitigation byte used by Hit_ResolveDamage, not padding
    /* 0x00522 */ u8 enemyFlags; /* bit 0: HUD name consumed/suppressed; bit 1: skips drops/collision; bit 2: force player target */
    /* 0x00523 */ u8 floorFlags; /* last floor tile flags (Entity_MoveWithCollisionProfile; 0xC0 = slope) */
    /* 0x00524 */ union EntityPosX posX; // 16.16
    /* 0x00528 */ union EntityPosY posY; // 16.16
    /* 0x0052C */ union EntityVelX velX; // 16.16, added to posX by EntityMove
    /* 0x00530 */ union EntityVelY velY; // 16.16, added to posY by EntityMove
    /* 0x00534 */ union EntityAccelX accelX; // 16.16, added to velX by EntityAccelerate*
    /* 0x00538 */ union EntityAccelY accelY; // 16.16, added to velY (gravity); some effects reuse both as an offset or half size
    /* The u16 render fields preserve agbcc's temporary modes for direct slot
     * and flag stores, replacing per-function halfword views. Their packed bit
     * positions and byte layout are unchanged. This matching type model does
     * not prove the original declarations. */
    /* 0x0053C */ u16 affineIndex:5;
    /* 0x0053C */ u16 hidden:1;
    /* 0x0053C */ u16 flipX:1; // todo: why u16? Mirrors boxes/rects horizontally (Entity_ApplyFlipToRect32)
    /* 0x0053C */ u8 flipY:1; // mirrors boxes/rects vertically (Entity_ApplyFlipToRect32)
    /* 0x0053D */ u8 animDone:1;
    /* 0x0053D */ u8 scrollWithBg:1; /* moved by Entity_ScrollWithBg */
    /* 0x0053D */ u8 roomActive:1;
    /* 0x0053D */ u16 deleteRequested:1; /* freed after its update */
    /* 0x0053D */ u16 affine:1;
    /* 0x0053D */ u8 doubleSize:1;
    /* 0x0053D */ u8 shrinkTransformedOffsets:1; /* multiply transformed offsets by 0xF6/0x100 */
    /* 0x0053D */ u8 shrinkPartOffsets:1; /* multiply part coordinates by 0xE8/0x100 before transforming */ // set independently by shrinking sprites and beams
    /* 0x0053E */ u8 mosaic:1;
    /* 0x0053E */ u8 objMode:2;
    /* 0x0053E */ u16 palette:4;
    /* 0x0053E */ u8 unk_53E_7:1; // Real?
    /* 0x0053F */ u8 animMode; // Type?
    /* 0x00540 */ u8 spriteLayer;
    /* 0x00541 */ u8 pad_541[0x546 - 0x541];
    /* 0x00546 */ u8 paletteMask;
    /* 0x00547 */ u8 group; /* EntityDeleteGroup */
    /* 0x00548 */ u8 spriteSlot;
    /* 0x00549 */ u8 animFrame;
    /* 0x0054A */ u8 pad_54A[0x54C - 0x54A];
    /* 0x0054C */ struct AnimScript *animScript;
    /* 0x00550 */ u8 animFlags;
    /* 0x00551 */ u8 animScriptIndex;
    /* 0x00552 */ u8 animStep;
    /* 0x00553 */ u8 animTimer;
    /* 0x00554 */ struct EntityHit hit;
}; /* size = 0x84 */

struct BgMapMetadata {
    u8 widthCells; // 256-pixel cells; lookup retains special single-cell viewport bounds
    u8 heightCells;
    u16 formatFlags; // bit 0 affine fill, bit 1 compressed/cached tile and collision data
    u16 *tileData; // raw entries or compressed resource, according to formatFlags
    u8 *collisionData; // optional tile collision flags/resource, used by BG1
    u16 *blockMap; // 4x4-tile block indices with horizontal/vertical flip bits
};

struct CameraFrac8 {
    u8 fraction0;
    u8 fraction1;
    u8 integer0;
    u8 integer1;
};

struct CameraFrac16 {
    u16 fraction;
    u16 integer;
};

union CameraPart {
    struct CameraFrac8 part8;
    struct CameraFrac16 part16;
    s32 whole;
};

struct BgLayerState {
    /* 0x00 */ struct BgMapMetadata *pBgMetadata;
    /* 0x04 */ union CameraPart xPos;
    /* 0x08 */ union CameraPart yPos;
    /* 0x0C */ union CameraPart xVel;
    /* 0x10 */ union CameraPart yVel;
    /* 0x14 */ u16 pixelWidth;
    /* 0x16 */ u16 pixelHeight;
    /* 0x18 */ u8 scrollMode;
    /* 0x19 */ u8 pad_19[0x1C - 0x19];
};

struct SolidObjectCallbackStorage {
    u32 flags;
    void *ceilingCorrection;
    void *floorCorrection;
    void *wallRightCorrection;
    void *wallLeftCorrection;
    void *push;
};

struct EwramData_unk1316C {
    u8 pad_0[0x10 - 0x0];
    struct SolidObjectCallbackStorage *unk_10;
};

struct SavedDisplayState {
    /* 0x00068 */ u16 displayControl;
    /* 0x0006A */ u16 blendControl;
    /* 0x0006C */ u16 blendAlpha;
    /* 0x0006E */ u8 vcountDmaMode:7;
    /* 0x0006E */ u8 hBlankRequestStart:1;
    /* 0x0006F */ u8 hBlankVcountSetting;
    /* 0x00070 */ u8 bgControlBytes[0x8];
}; /* size = 0x10 */

/* Existing two-byte prefix view, cast to full map metadata by Room_Load. */
struct RoomBgDimensions {
    u8 widthCells;
    u8 heightCells;
};

struct RoomBgLayerDefinition {
    u8 layer; // logical 0..31 BG ordering passed to Obj_SetBgLayer
    u8 scrollMode;
    u16 control; // BGCNT value for this room layer
    u16 pixelWidth;
    u16 pixelHeight;
    struct RoomBgDimensions *dimensions;
};

struct RoomResourceLoad {
    u32 *resource; // graphics/palette resource; retain the declared pointer type
    u8 destinationIndex;
    u8 sourceIndex;
    u8 count;
    /* Indices/count use 0x800-byte graphics blocks or palette units,
     * depending on the consuming loader. */
};

struct RoomTransitionPart {
    struct RoomDefinition *destination;
    s8 cellX;
    s8 cellY;
    u16 entryXOffset;
    u16 unk_8; // Type?
    u16 cameraX;
    u16 cameraY;
    u16 unk_E; // Type?
};

struct RoomDefinition {
    u16 displayControl; // DISPCNT value restored by room/display loading
    u16 definitionChainMarker; // Room_Load follows nextDefinition until this marker is 0xFFFF; broader meaning unresolved
    struct RoomDefinition *nextDefinition;
    struct RoomBgLayerDefinition *bgLayers;
    struct RoomResourceLoad *graphicsLoads;
    struct RoomResourceLoad *paletteLoads;
    void *entityRecords; // RoomEntity_InitList consumes this as RoomEntityRecord; retain void * declaration
    struct RoomTransitionPart *roomParts;
    u8 pad_1C[0x1E - 0x1C];
    u16 blendControl; // BLDCNT value restored after room loading
    u8 pad_20[0x22 - 0x20];
    u16 mapX:7; // room origin in map cells
    u16 mapY:7;
    u16 unk_23_0:1;
    u16 collisionBlocksEnabled:1;
};

/* Two bits per map cell: gameState.mapCells[x >> 5][y], bit x & 31 (64 x 40
 * grid). Map_MarkCurrentCell sets both bits of the player's cell. */
struct MapCellBits {
    /* 0x000B4 */ u32 visitedThisRun; /* MapScreen_DrawMap draws room-type cells (save, warp) from this plane */
    /* 0x000B8 */ u32 visited; /* counted by Map_GetCompletionTenths */
}; /* size = 0x8 */

/* Twelve-byte room-list record. Coordinates are room pixels; x == 0x7FFF is
 * the list terminator. kind selects the spawn path; entityId and the two
 * spawn parameters have kind/entity-specific meanings. */
struct RoomEntityRecord {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ u8 spawnIndex; // bit index in roomEntitySpawn.spawnedFlags
    /* 0x5 */ u8 kind;
    /* 0x6 */ u8 entityId;
    /* 0x7 */ u8 flags; // low nibble requests spawning outside the initial camera window
    /* 0x8 */ u16 spawnParam0;
    /* 0xA */ u16 spawnParam1;
}; /* size = 0xC */

struct RoomEntitySpawnState {
    /* 0x003D0 */ u8 scanAxis; // 0 = X, 1 = Y; selected from room dimensions
    /* 0x003D1 */ u8 scanIndex; // cursor updated as the camera scrolls
    /* 0x003D2 */ u8 pad_2[0x4 - 0x2];
    /* 0x003D4 */ s32 spawnedFlags[2]; // room-list indices already spawned; retain signed storage
    /* 0x003DC */ struct RoomEntityRecord *records;
}; /* size = 0x10 */

/* Game state (gEwramData->gameState): system flags, the saved slot header
 * (0x94..0xB3), map cells, resume position and game flags (saved with the
 * slot), and the runtime room, message and event state after them. */
struct GameState {
    /* 0x00060 */ u32 systemFlags; /* written to SRAM system slot 6 (code_08012744.c) */
    /* 0x00064 */ u8 subMode; // dispatcher in GameModeInGameUpdate, distinct from the top-level game mode
    /* 0x00065 */ u8 subModeStep; // initialization/menu/warp/transition state within the selected sub-mode
    /* 0x00066 */ u16 subModeCounter; // shared up/down counter: delays, link sync and banner/fade progress
    /* 0x00068 */ struct SavedDisplayState savedDisplay;
    /* 0x00078 */ u8 savedWindowRegisters[0x10];
    /* 0x00088 */ struct RoomDefinition* currentRoomDefinition;
    /* 0x0008C */ u16 roomMapXPos:7;
    /* 0x0008C */ u16 roomMapYPos:7;
    /* 0x0008D */ u16 unk_8D_6:2;
    /* 0x0008E */ u8 unk_8E_0:1;
    /* 0x0008E */ u8 unk_8E_1:3; // Length?
    /* 0x0008E */ u8 spawnCameraOffsetMode:2;
    /* 0x0008E */ u8 bestiaryPreview:1; // set by BestiaryMenu_Open: enemies spawn as animation-only previews
    /* 0x0008E */ u8 unk_8E_7:1;
    /* 0x0008F */ u8 pad_8F[0x90 - 0x8F];
    /* 0x00090 */ u16 currentSong;
    /* 0x00092 */ u8 cachedPickupIcon; // icon cached in OBJ VRAM by PickupSource_LoadIcon, 0xFF = none
    /* 0x00093 */ u8 pad_93;
    /* Slot header (0x94..0xB3, see struct SaveHeader). */
    /* 0x00094 */ u8 *playerName; /* 8 bytes of name text; declared as a pointer (matching type model) */
    /* 0x00098 */ u8 pad_98[0x9E - 0x98];
    /* 0x0009E */ u8 currentArea;
    /* 0x0009F */ u8 currentRoom;
    /* 0x000A0 */ u8 saveLevel; /* playerStatus.currentLevel at save time */
    /* 0x000A1 */ u8 gameCleared:4; /* set by SaveData_ResaveSlotAfterEnding after a clear */
    /* 0x000A1 */ u8 difficultyMode:4;
    /* 0x000A2 */ u8 slotLinks[2]; /* [0] source slot + 1 of a copy, [1] copy slot + 1 of a source */
    /* 0x000A4 */ u8 saveCharacter; /* playerStatus.currentCharacter at save time */
    /* 0x000A5 */ u8 unk_A5; /* playerStatus.unk_13267 at save time */
    /* 0x000A6 */ u16 saveMapCompletionTenths; /* Map_GetCompletionTenths at save time */
    /* 0x000A8 */ u32 saveGold; /* playerStatus.currentGold at save time */
    /* 0x000AC */ u32 inGameTimer;
    /* 0x000B0 */ u32 bestBossRushTimeFrames; // replaced by entity 26 unk_4F8 when 0 or larger (Object10Update)
    /* 0x000B4 */struct MapCellBits mapCells[2][40];
    /* Resume position, stored when saving and used by the load (Room_LoadAtCamera). */
    /* 0x00334 */ u16 savedCameraX;
    /* 0x00336 */ u16 savedCameraY;
    /* 0x00338 */ s16 savedPlayerX;
    /* 0x0033A */ s16 savedPlayerY;
    /* 0x0033C */ u32 gameFlags[2]; // start of the 256-bit flag array of GameFlag_Set/_Clear/_TestAndSet (continues into gameFlagsTail)
    /* 0x00344 */ u32 gameFlagsTail[6]; // flag words 2..7, indexed directly by Object35Create
    /* 0x0035C */ u8 pad_35C[0x360 - 0x35C];
    /* 0x00360 */ u32 collectedPickupFlags[1]; // room pickup bits; original one-element declaration retained
    /* 0x00364 */ u8 pad_364[0x378 - 0x364];
    /* 0x00378 */ u32 activatedWarpFlags[1]; // warp-table indices; selection uses bits 0..7
    /* 0x0037C */ u16 shownAreaNameFlags; // banners already shown, including area-name overrides 12..15
    /* 0x0037E */ u16 bossEventFlags; // boss/recovery events; gates doors, respawns, music and scenes
    /* 0x00380 */ u8 unk_380;
    /* 0x00381 */ u8 pad_381[0x3CC - 0x381];
    /* 0x003CC */ u32 *roomData; /* GetRoomPointer(currentArea, currentRoom) */
    /* 0x003D0 */ struct RoomEntitySpawnState roomEntitySpawn;
    /* 0x003E0 */ s32 sharedObjPaletteSources[4]; // pointer bits in s32 storage; hardware palette banks 10..13
    /* 0x003F0 */ u8 sharedObjPaletteVariants[4]; // variant paired with the corresponding palette source
    /* 0x003F4 */ s8 selectedWarpIndex; // 0..7, or -1 when the warp menu is cancelled
    /* 0x003F5 */ u8 pad_3F5[0x3F8 - 0x3F5];
    /* 0x003F8 */ struct RoomDefinition *roomJumpDestination; // installed by RoomJump_SetDestination
    /* 0x003FC */ u16 roomJumpCameraX;
    /* 0x003FE */ u16 roomJumpCameraY;
    /* 0x00400 */ s16 roomJumpPlayerX;
    /* 0x00402 */ s16 roomJumpPlayerY;
    /* HUD gauge caches approach the corresponding playerStatus values gradually. */
    /* 0x00404 */ s16 hudHP;
    /* 0x00406 */ s16 hudMP;
    /* 0x00408 */ s16 hudMaxHP;
    /* 0x0040A */ s16 hudMaxMP;
    /* 0x0040C */ u8 *conversationText; // conversation text (Conversation_LoadNextText)
    /* 0x00410 */ u32 insertedTextAddress; // pointer bits for command 1's nested stream; retain u32 storage
    /* 0x00414 */ u8 textWindowState:4; // set to 5 by Object20Update (newInput bit 3)
    /* 0x00414 */ u8 textWindowLabelId:4;
    /* 0x00415 */ u8 textCharacterTimer; // active countdown in textWindowState 4; not padding
    /* 0x00416 */ u8 textCharacterDelay; // delay reloaded after a character; zero enables fast text
    /* 0x00417 */ u8 textWindowPause; // nonzero pauses parser states 0..4; TextWindow_Close writes 255
    /* 0x00418 */ u16 conversationTextOffset; // offset in the main command stream (past its two-byte header)
    /* 0x0041A */ u8 insertedTextOffset; // nested-stream offset; retain its original narrower type
    /* 0x0041B */ u8 textWindowOpening; // stored amount; TextWindow_AdjustOpening clamps only a local copy
    /* 0x0041C */ struct EwramData_EntityData *sceneController; // set to the object by Object20Create
    /* 0x00420 */ u16 hudMessageId; // request text ID; the soul-acquired path also stores its soul ID
    /* 0x00422 */ u8 hudMessageTimer; // close the HUD message when this countdown reaches zero
    /* 0x00423 */ u8 hudMessageRequest; // 0 none, 1 pickup text, 2 enemy name, 3 close
    /* 0x00424 */ u8 hudTextBufferIndex; // alternating Text8 row/tile buffer, toggled after display
    /* 0x00425 */ u8 pad_425[0x427 - 0x425];
    /* 0x00427 */ u8 cameraScrollMinPriority; // Camera_ScrollWithPriority rejects lower-priority callers
    /* 0x00428 */ u8 currentSaveSlot; // 0: File 1, 1: Quicksave 1, 2: File 2, 3: Quicksave 2, 4: File 3, 5: Quicksave 3
    /* 0x00429 */ u8 pad_429[0x42C - 0x429];
    /* 0x0042C */ u32 stateFlags; /* runtime flags: bit 0 event running, 2 boss fight, 0x10 player dead,
                                   * 0x20 HUD hidden, 0x80 banner, 0x200 HUD messages blocked,
                                   * 0x01000000 large message window open, 0x02000000 window closed */
    /* 0x00430 */ u8 pad_430[0x4BE - 0x430];
    /* 0x004BE */ s16 timeStopTimer; // Chronomage countdown; nonzero pauses affected entities
    /* 0x004C0 */ u8 screenShakePriority;
    /* 0x004C1 */ u8 screenShakeTimer;
    /* 0x004C2 */ s8 screenShakeComponentX;
    /* 0x004C3 */ s8 screenShakeComponentY;
    /* 0x004C4 */ s32 conveyorSpeed; // initialized to -0x8000 by Room_ResetTransientState
    /* 0x004C8 */ u32 bgEffectFrameCounter;
    /* 0x004CC */ u8 bossRushMode:2; // Boss Rush entry sets 1; retain the original two-bit field
    /* 0x004CC */ u8 areaNameOwnsHBlank:1;
    /* 0x004CC */ u8 unk_4CC_3:3; // More fields?
    /* 0x004CC */ u8 linkPlay:1; /* link play: partner player sprite, enemies target playerState.linkTargetProxy */
    /* 0x004CD */ u8 areaNameState;
    /* 0x004CE */ u8 pad_4CE[0x4D0 - 0x4CE];
    /* 0x004D0 */ u32 language; // 1 = English, 2 = French, 3 = German
}; /* size = 0x474 */

/*
 * Save slot layout in SRAM (SaveData_*, code_08012744.c): each slot is
 * 0x47C bytes at SRAM + 0x10 + slot * 0x47C:
 *   +0x000 gEwramData->playerStatus (0x190, struct PlayerStatus),
 *   +0x190 gameState.playerName .. bestBossRushTimeFrames (0x20, the slot header),
 *   +0x1B0 gameState.mapCells .. (0x2CC, map and game flags).
 * The menus read only the header of a slot into gEwramData->saveHeaders[i]
 * (SaveData_ReadSlotHeader); its layout mirrors gameState 0x94..0xB3, whose
 * summary fields are refreshed from playerStatus before every save.
 */
struct SaveHeader {
    /* 0x00 */ u8 name[8]; /* player name (gameState.playerName) */
    /* 0x08 */ u8 pad_28[0x2A - 0x28];
    /* 0x0A */ u8 area; /* gameState.currentArea */
    /* 0x0B */ u8 room; /* gameState.currentRoom */
    /* 0x0C */ u8 level; /* gameState.saveLevel */
    /* 0x0D */ u8 clearAndDifficulty; /* low nibble = gameState.gameCleared, high = gameState.difficultyMode */
    /* 0x0E */ u8 sourceSlotLink; /* gameState.slotLinks[0] */
    /* 0x0F */ u8 copySlotLink; /* gameState.slotLinks[1] */
    /* 0x10 */ u8 character; /* gameState.saveCharacter */
    /* 0x11 */ u8 unk_31; /* gameState.unk_A5 */
    /* 0x12 */ u16 mapCompletionTenths; /* gameState.saveMapCompletionTenths */
    /* 0x14 */ u32 gold; /* gameState.saveGold */
    /* 0x18 */ u32 playTime; /* frames (gameState.inGameTimer); 0 = slot unused */
    /* 0x1C */ u32 bestBossRushTimeFrames; /* gameState.bestBossRushTimeFrames, shown as a second time once cleared */
}; /* size = 0x20 */

struct SoulTradePacket {
    /* 0x25484 */ u8 offeredSoulKind:2;
    /* 0x25484 */ u8 acknowledgedSoulKind:2;
    /* 0x25484 */ u8 handshakeFlags:4;
    /* 0x25485 */ u8 soulIds[2];
    /* 0x25487 */ u8 packetTag;
    /* 0x25488 */ u8 pad_25488[0x25494 - 0x25488];
}; /* size = 0x10 */

/* Player state (gEwramData->playerState): the player entity, its weapon
 * and soul entities, stats and per-character data. */
struct PlayerState {
    /* 0x13110 */ struct EwramData_EntityData *player; /* entity slot 0, Player_Create */
    /* 0x13114 */ struct EwramData_EntityData *linkPartner;
    /* 0x13118 */ struct EwramData_EntityData *linkTargetProxy;
    /* 0x1311C */ struct EwramData_EntityData *weaponEntities[1]; /* hit entity of weapon slot 0/1 (whip strike, sub weapon) */
    /* 0x13120 */ struct EwramData_EntityData *whipChain; /* Whip_CreateChain */
    /* 0x13124 */ struct EwramData_EntityData *activeRedSoulEntities[0x8];
    /* 0x13144 */ struct EwramData_EntityData *activeBlueSoulEntities[0x8];
    /* 0x13164 */ struct EwramData_EntityData *activeYellowSoulEntity; // see YellowSoul_CreateEquippedEntity / YellowSoul_Stop
    /* 0x13168 */ struct EwramData_EntityData *enemyTarget; /* entity enemies aim at: the player or a decoy (SkillKillerDollUse, SkillNemesisUse) */
    /* 0x1316C */ struct EwramData_EntityData *solidObject; /* Player_RegisterSolidObject: object whose callbacks add collision */
    /* 0x13170 */ struct EwramData_EntityData *collisionQueueEntities[1]; /* seven-entry use; original extent/padding retained */
    /* 0x13174 */ u8 pad_13174[0x13190 - 0x13174];
    /* 0x13190 */ struct EwramData_EntityData *standingPlatform; /* platform entity the player stands on */
    /* 0x13194 */ struct EwramData_EntityData *trackedEntities[4];
    /* 0x131A4 */ u8 trackedEntityRefreshCounters[8]; /* pairs: previous sample, current refresh count */
    /* 0x131AC */ u8 collisionQueueFlags[1]; /* seven-entry use; original extent/padding retained */
    /* 0x131AD */ u8 pad_131AD[0x131B4 - 0x131AD];
    /* 0x131B4 */ u8 collisionQueueCount;
    /* 0x131B5 */ u8 pad_131B5[0x131B8 - 0x131B5];
    /* 0x131B8 */ u32 actionLockFlags;
    /* 0x131BC */ u16 unusedHalfword; /* never written; read only by the unreferenced PlayerState_GetUnusedHalfword */
    /* 0x131BE */ u8 nbrActiveRedSoulEntities;
    /* 0x131BF */ u8 lastUsedRedSoul;
    /* 0x131C0 */ u8 cachedWeaponGraphicsIndices[1]; // graphics table keys for slots 0/1; index 1 aliases cachedWeaponGraphicsIndex1
    /* 0x131C1 */ u8 cachedWeaponGraphicsIndex1;
    /* 0x131C2 */ u16 skillBaseAttack;
    /* 0x131C4 */ u16 skillAttackAttributes;
    /* 0x131C6 */ u8 pad_131C6[0x131C8 - 0x131C6];
    /* 0x131C8 */ u32 weaponGfx; /* ObjGfx slot of the weapon slot 0 graphics, -1 = none */
    /* 0x131CC */ u32 redSoulGfx; /* ObjGfx slot of the red soul / weapon slot 1 graphics, -1 = none */
    /* 0x131D0 */ u32 blueSoulGfx; /* ObjGfx slot of the blue soul graphics */
    /* 0x131D4 */ u16 hitReaction;
    /* 0x131D6 */ u16 pendingHitDamage;
    /* 0x131D8 */ u32 hitSourceX;
    /* 0x131DC */ u32 hitSourceY;
    /* 0x131E0 */ s32 knockbackVelocityX; /* 16.16 velocity applied for hitReaction 3 */
    /* 0x131E4 */ s32 knockbackVelocityY; /* 16.16 velocity applied for hitReaction 3 */
    /* 0x131E8 */ u8 pad_131E8[0x131EA - 0x131E8];
    /* 0x131EA */ u16 unk_131EA;
    /* 0x131EC */ u16 unk_131EC;
    /* 0x131EE */ s16 currentATK;
    /* 0x131F0 */ s16 currentDEF;
    /* 0x131F2 */ s16 currentStats[4]; // 0: STR, 1: CON, 2: INT, 3: LCK
    /* 0x131FA */ u16 resistances; /* attack-type bits that halve damage (souls, equipment) */
    /* 0x131FC */ s16 equipmentAttackBonus;
    /* 0x131FE */ s16 equipmentDefenseBonus;
    /* 0x13200 */ u16 preparedStats[4];
    /* 0x13208 */ u16 preparedResistances;
    /* 0x1320A */ u8 pad_1320A[0x1320C - 0x1320A];
    /* 0x1320C */ s32 cachedPlayerX; /* post-collision 16.16 cache; number-indicator X source */
    /* 0x13210 */ s32 cachedPlayerY; /* post-collision 16.16 cache; no direct consumer found */
    /* 0x13214 */ s16 cameraAnchorX; /* desired player screen X, pixels */
    /* 0x13216 */ s16 cameraAnchorY; /* desired player screen Y, pixels */
    /* 0x13218 */ u8 wallProbeCount; /* count followed by four signed-byte Y offsets */
    /* 0x13219 */ u8 wallProbeY0;
    /* 0x1321A */ u8 wallProbeY1;
    /* 0x1321B */ u8 wallProbeY2;
    /* 0x1321C */ u8 wallProbeY3;
    /* 0x1321D */ u8 pad_1321D[0x13220 - 0x1321D];
    /* 0x13220 */ struct EwramData_EntityData *selectedTarget; /* shared chosen target; Player_SetSelectedTarget */
    /* 0x13224 */ u16 linkFrameCounter;
    /* 0x13226 */ s8 linkDisconnectCounter;
    /* 0x13227 */ u8 pad_13227[0x13228 - 0x13227];
    /* 0x13228 */ u16 linkHistoryX[1]; /* eight-entry use; original extent/padding retained */
    /* 0x1322A */ u8 pad_1322A[0x13238 - 0x1322A];
    /* 0x13238 */ u16 linkHistoryY[1]; /* eight-entry use; original extent/padding retained */
    /* 0x1323A */ u8 pad_1323A[0x13248 - 0x1323A];
    /* 0x13248 */ u16 linkSampleX;
    /* 0x1324A */ u16 linkSampleY;
    /* 0x1324C */ u8 pad_1324C[0x1325C - 0x1324C];
}; /* size = 0x14C */

/* Saved player data (gEwramData->playerStatus, the first 0x190 bytes of a
 * save slot): equipment, inventories, level, HP/MP, stats, status effects,
 * button config and bestiary flags. */
struct PlayerStatus {
    /* 0x1325C */ u32 inputOverrides; /* bit 0 acts as Down held, bit 1 as attack pressed; cleared after the player update */
    /* 0x13260 */ u32 soulStateFlags; /* active soul states (Player_TestSoulStateFlags), e.g. 0x200 bat form */
    /* 0x13264 */ u16 totalNbrSoulsCollected;
    /* 0x13266 */ u8 currentCharacter; // 0: Soma, >= 1: Julius 
    /* 0x13267 */ u8 unk_13267;
    /* 0x13268 */ u8 equippedWeapon;
    /* 0x13269 */ u8 equippedRedSoul;
    /* 0x1326A */ u8 equippedBlueSoul;
    /* 0x1326B */ u8 equippedYellowSoul;
    /* 0x1326C */ u8 equippedArmor;
    /* 0x1326D */ u8 equippedAccessory;
    /* 0x1326E */ u8 pad_1326E[0x13270 - 0x1326E];
    /* 0x13270 */ u16 statusEffects; /* bits 0..2: status effects set by Player_SetStatusEffects; bit 0 quarters STR/CON/INT, bit 2 petrified (double damage) */
    /* 0x13272 */ s16 statusEffectTimers[2]; /* countdowns of effects 0 and 1; index 2 is petrifyCounter */
    /* 0x13276 */ s16 petrifyCounter; /* lowered by button presses; the stone breaks at 0 */
    /* 0x13278 */ u8 pad_13278[0x13279 - 0x13278];
    /* 0x13279 */ u8 currentLevel;
    /* 0x1327A */ s16 currentHP;
    /* 0x1327C */ s16 currentMP;
    /* 0x1327E */ u16 maxHP;
    /* 0x13280 */ u16 maxMP;
    /* 0x13282 */ u16 baseStats[4]; // 0: STR, 1: CON, 2: INT, 3: LCK
    /* 0x1328A */ u8 pad_1328A[0x1328C - 0x1328A];
    /* 0x1328C */ u32 currentExperience;
    /* 0x13290 */ u32 currentGold;
    /* 0x13294 */ u8 itemInventory[0x20];
    /* 0x132B4 */ u8 weaponInventory[0x3B];
    /* 0x132EF */ u8 armorInventory[0x19];
    /* 0x13308 */ u8 accessoryInventory[0x14];
    /* 0x1331C */ u8 redSoulInventory[2][0x1C];
    /* 0x13354 */ u8 blueSoulInventory[2][0xD];
    /* 0x1336E */ u8 yellowSoulInventory[2][0x12];
    /* 0x13392 */ u8 abilitySoulInventory[0x3];
    /* 0x13395 */ u8 pad_13395[0x13396 - 0x13395]; // TODO: is abilitySoulInventory really 4 entries?
    /* 0x13396 */ u8 abilityFlags[0x2]; /* bit per ability index (code_08032444.c) */
    /* 0x13398 */ u16 attackButton;
    /* 0x1339A */ u16 jumpButton;
    /* 0x1339C */ u16 abilityButton;
    /* 0x1339E */ u16 guardianButton;
    /* 0x133A0 */ u32 enemyFlags[12]; /* bit per enemy id: [0..3] defeated (Bestiary_IsEnemyDefeated), [4..7] and [8..11] drops 1 and 2 obtained */
    /* 0x133D0 */ u8 weaponList[4];
    /* 0x133D4 */ u8 armorList[4];
    /* 0x133D8 */ u8 accessoryList[4];
    /* 0x133DC */ u8 redSoulList[4]; // kinds 0..2 as in SoulMenu_PrintEquippedName (red/blue/yellow)
    /* 0x133E0 */ u8 blueSoulList[4];
    /* 0x133E4 */ u8 yellowSoulList[4];
    /* 0x133E8 */ u8 pad_133E8[0x133EC - 0x133E8];
}; /* size = 0x190 */

/*
 * OBJ graphics cache (ObjGfx_*, code_08039340.c .. code_0803B66C.c).
 *
 * gEwramData->objGfx holds 16 entries in two doubly linked lists (free list
 * objGfxFreeHead, used list objGfxUsedHead). Each used entry records one
 * graphics resource loaded into a run of OBJ tile slots (ObjTiles_*), so a
 * resource that is already in VRAM is found by ObjGfx_Find and shared.
 *
 * Graphics resource header: [0] type - 0 raw tiles at +4, 1 LZ77-compressed
 * (pointer at +4, unpacked into gfxDecompBuffer), 2 frame list (pointers to
 * type 0/1 resources at +4, [2] = slots per frame, [3] = frame count),
 * 3 frame set (decompressed into objGfxFrameBuffers by ObjGfx_DecompressSet);
 * [3] of a type 0/1 resource is its height in tile rows (4, 8 or 16).
 * Tiles are copied as rows of 16 tiles (32 in 8bpp) into the 2D OBJ layout.
 */
#define OBJ_GFX_KIND_SINGLE 0 /* one type 0/1 resource */
#define OBJ_GFX_KIND_FRAMES 1 /* type 2 frame list; frames[] picks the loaded ones */
#define OBJ_GFX_KIND_SET 2    /* type 3 frame set via objGfxSetFrames */

struct ObjGfxEntry {
    /* 0x0FEC4 */ s16 prev; /* list links (entry indices, -1 = none) */
    /* 0x0FEC6 */ s16 next;
    /* 0x0FEC8 */ s32 tileSlot; /* first ObjTiles slot, -1 = entry unused */
    /* 0x0FECC */ u8 kind; /* OBJ_GFX_KIND_* */
    /* 0x0FECD */ u8 lifetime; /* 0 = dropped by ObjGfx_SaveState and reloaded
                                * by ObjGfx_RestoreState, 2 = loaded while
                                * saved (freed on restore), else kept */
    /* 0x0FECE */ u8 size; /* slots per frame */
    /* 0x0FECF */ u8 count; /* frames loaded side by side */
    /* 0x0FED0 */ u8 frames[4]; /* source frame index shown in each loaded frame */
    /* 0x0FED4 */ void *source; /* the loaded resource (lookup key) */
}; /* size = 0x14 */

/*
 * Entity sprites (Sprite_*, code_08039340.c .. code_0803B66C.c).
 *
 * An entity with a sprite owns one of the 240 gEwramData->spriteSlots
 * (entity->spriteSlot, 0xFF = none). The slot links the entity's sprite
 * sheet (animation data) with the tile slot of its graphics (ObjGfx_*).
 * Each frame Sprite_Draw turns the current SpriteFrame (entity->animFrame)
 * into one OAM object per SpritePart, through the ARM copies of
 * Sprite_DrawParts / Sprite_DrawPartsAffine (entity->affine selects the
 * affine one, which uses the affine set entity->affineIndex).
 */

/* Hitbox of a sprite frame: offset from the entity position (mirrored by
 * the entity's flips) and size in pixels. */
struct SpriteBox {
    s8 x;
    s8 y;
    u8 width;
    u8 height;
};

/* One OAM object of a sprite frame. */
struct SpritePart {
    s8 x; /* position relative to the entity */
    s8 y;
    u8 unk_2;
    u8 unk_3;
    s8 tileX; /* pixel position of the tiles inside the tile slot run */
    s8 tileY;
    u8 width;
    u8 height;
    u8 shape:4; /* OAM shape and size */
    u8 size:4;
    u8 gfxFrame; /* nonzero: tiles of the second loaded graphics frame */
    u8 flip; /* bit 0: toggle vertical, bit 1: toggle horizontal flip */
    u8 paletteOffset; /* added to entity->palette (masked by paletteMask) */
};

struct SpriteFrame {
    s8 originX;
    s8 originY;
    u8 pad_2[0x4 - 0x2];
    u8 boxCount;
    u8 partCount; // sprite part count (Sprite_GetPartCornerOffset / Sprite_GetPartCenterOffset / Sprite_GetPartOriginOffset)
    u8 pad_6[0x8 - 0x6];
    struct SpriteBox *boxes;
    // u8 pad_C[0x10 - 0xC];
    struct SpritePart *parts;
};

/* Animation data of a sprite: frames and the animation scripts. */
struct SpriteSheet {
    u16 frameCount;
    u16 scriptCount;
    struct SpriteFrame *frames;
    struct AnimScript *scripts;
    struct AnimScript **scriptTable;
};

struct SpriteSlot {
    s16 prev;
    s16 next;
    s32 gfxEntry;
    s32 tileSlot;
    struct SpriteSheet *sheet;
};

struct ObjGfxFrameResource {
    u8 kind;
    u8 bitsPerPixel;
    u8 widthTiles;
    u8 heightTiles;
    void *compressedData;
}; /* size = 0x8*/

struct EwramData_unk19404 {
    u8 unk_0;
    u8 unk_1;
    u8 unk_2;
    u8 unk_3;
    struct ObjGfxFrameResource *unk_4[1]; // TODO: should this be a struct EwramData_unk19404* ?
    u8 pad_8[0x2010 - 0x8];
}; /* size = 0x2010 */

struct LinkSendPlayerRecord {
    /* 0x004D4 */ u16 roomX;
    /* 0x004D6 */ u16 roomY;
    /* 0x004D8 */ u8 animFrame;
    /* 0x004D9 */ u8 flags;
    /* 0x004DA */ u16 frameCounter;
}; /* size = 0x8 */

struct LinkReceivedPlayerRecord {
    /* 0x004DC */ u16 roomX;
    /* 0x004DE */ u16 roomY;
    /* 0x004E0 */ u8 animFrame;
    /* 0x004E1 */ u8 flags;
    /* 0x004E2 */ u16 frameCounter;
}; /* size = 0x8 */

/*
 * Palette system (code_0803B66C.c, code_0803C294.c, code_0803C49C.c).
 *
 * Two PaletteBuffers live in EWRAM: objPalette (uploaded to OBJ palette RAM)
 * and bgPalette (uploaded to BG palette RAM). Each holds 16 palettes of 16
 * BGR555 colors in three copies:
 *   base   - palettes as loaded by the game (ObjPalette_LoadRange etc.),
 *   saved  - backup taken by *_SaveSlot/*_SaveAll, restored by *_Restore*,
 *   output - base after fades are applied; this copy is DMA'd to palette RAM.
 *
 * Every frame Palette_Update runs the palette animations (into objPalette
 * base), computes output from base and the fade state (Palette_UpdateFades)
 * and applies the invert effect; Palette_TransferToPalram uploads output in
 * VBlank when 'uploaded' was cleared.
 *
 * Fades blend each color as  base * w + target * (1 - w)  with w a 16.16
 * weight (0x10000 = base unchanged). Slots 0-15 blend palette N toward a
 * target palette (*_StartBlend); slot PALETTE_FADE_COLOR (16) blends every
 * palette toward one solid color (*_StartColorFade). Each slot moves its
 * weight by 'step' per frame until it reaches 'targetWeight'.
 */

/* Fade slot index of the solid-color fade (bit 16 of the fade masks). */
#define PALETTE_FADE_COLOR 16

/* Palette resource header: [2] = number of palettes, [3] = default
 * destination slot, followed by the palette data (0x20 bytes each) at +4. */
#define PALETTE_RESOURCE_COUNT(res) ((res)[2])
#define PALETTE_RESOURCE_SLOT(res) ((res)[3])

/*
 * Palette animation: cycles palettes of a resource into one objPalette slot.
 * Script: u16 frame count, then 4-byte frames {u8 resource palette index,
 * u8 duration in frames, 2 unused} starting at +4.
 */
struct PaletteAnimSlot {
    /* 0x0 */ u8 *palette;
    /* 0x4 */ u8 *script;
    /* 0x8 */ s16 timer;
    /* 0xA */ u8 frame;
    /* 0xB */ u8 pad_B;
}; /* size = 0xC */

struct PaletteAnimState {
    /* 0x12E5C */ s32 savedMask; /* bits 0-15: anims[i] saved in savedAnims[i] */
    /* 0x12E60 */ struct PaletteAnimSlot anims[0x10];
    /* 0x12F20 */ struct PaletteAnimSlot savedAnims[0x10];
}; /* size = 0x184 */

struct PaletteFade {
    /* 0x608 */ u16 *target[0x10]; /* target palette of blend slot N */
    /* 0x648 */ u8 colorR; /* solid-color fade target, 0-31 each */
    /* 0x649 */ u8 colorG;
    /* 0x64A */ u8 colorB;
    /* 0x64B */ u8 pad_43;
    /* 0x64C */ u32 activeMask; /* bit N: slot N is applied to output */
    /* 0x650 */ u32 runningMask; /* bit N: slot N advances (clear = paused) */
    /* 0x654 */ u32 pendingMask; /* bit N: slot N has not reached targetWeight */
    /* 0x658 */ s32 weight[0x11]; /* 16.16 weight of base, per slot */
    /* 0x69C */ s32 targetWeight[0x11];
    /* 0x6E0 */ s32 step[0x11]; /* added to weight each frame */
}; /* size = 0x11C */

struct PaletteBuffer {
    /* 0x000 */ /* 0x11DDC */ u16 base[0x10][0x10];
    /* 0x200 */ /* 0x11FDC */ u16 saved[0x10][0x10];
    /* 0x400 */ /* 0x121DC */ u16 output[0x10][0x10];
    /* 0x600 */ /* 0x123DC */ u16 dirtyMask; /* palettes loaded since last upload */
    /* 0x602 */ /* 0x123DE */ u16 savedMask; /* palettes held in 'saved' (0xFFFF = all) */
    /* 0x604 */ /* 0x123E0 */ u8 fadeSaved; /* fade state is held in savedFade */
    /* 0x605 */ /* 0x123E1 */ u8 uploaded; /* 0 = output must be uploaded in VBlank */
    /* 0x606 */ /* 0x123E2 */ u8 pad_606[0x123E4 - 0x123E2];
    /* 0x608 */ /* 0x123E4 */ struct PaletteFade fade;
    /* 0x724 */ /* 0x12500 */ struct PaletteFade savedFade;
}; /* size = 0x840 */

/* OBJ tile slot allocator (code_0803C49C.c, ObjTiles_*): see the comment
 * above ObjTiles_AllocForward. Bits of gEwramData->objTileFlags: */
#define OBJ_TILES_GRID 1     /* slots in the 2D grid layout (default) */
#define OBJ_TILES_COLOR256 2 /* 8bpp graphics: 8 double-size slots */
#define OBJ_TILES_BITMAP 4   /* bitmap BG mode: upper half of OBJ VRAM only */

struct ObjTileSlot {
    /* 0x0 */ u8 used:1;
    /* 0x1 */ u8 size; /* slots per graphic of the run this slot belongs to */
    /* 0x2 */ u8 count; /* graphics in that run */
    /* 0x3 */ u8 pad_3;
    /* 0x4 */ u16 firstTile; /* OBJ tile number of the slot (ObjTiles_GetSlotTile) */
    /* 0x6 */ u8 unk_6;
    /* 0x7 */ u8 unk_7;
}; /* size = 0x8 */

/* Backup of the affine sets (ObjAffine_SaveState / ObjAffine_RestoreState). */
struct ObjAffineBackup {
    s32 usedMask; /* gObjState.affineUsedMask */
    s32 saved; /* nonzero while a backup is held */
    u8 data[0x500]; /* copy of gObjAffine[0x20] */
}; /* size = 0x508 */

struct DebugMenuWork {
    /* 0x254D0 */ u8 state;
    /* 0x254D1 */ u8 spawnCount;
    /* 0x254D2 */ u8 pad_254D2[0x254D6 - 0x254D2];
    /* 0x254D6 */ s16 selection;
    /* 0x254D8 */ u16 entryValues[1]; // TODO: length?
    /* 0x254DA */ u8 pad_254DA[0x254E0 - 0x254DA];
}; /* size = 0x10 */

struct EwramData {
    /* 0x00000 */ u32 frameCounter;
    /* 0x00004 */ u16 gameModeTimer;
    /* 0x00006 */ u16 gameModeCounter;
    /* 0x00008 */ u32 randomNumber;
    /* 0x0000C */ u32 randomNumber2;

    /* 0x00010 */ u8 gameMode;
    /* 0x00011 */ u8 gameModeUpdateStage;
    /* 0x00012 */ u8 gameModeSubStage;
    /* 0x00013 */ u8 drawListHead; /* first entity of the draw list, 0xFF = empty */
    /* 0x00014 */ struct InputData inputData;

    /* 0x00020 */ struct SaveHeader saveHeaders[2];

    /* 0x00060 */ struct GameState gameState;
    
    /* 0x004D4 */ struct LinkSendPlayerRecord linkSendState;
    /* 0x004DC */ struct LinkReceivedPlayerRecord linkReceivedState; /* separate eight-byte receive view; original declaration retained */
    /* 0x004E4 */ struct EwramData_EntityData entityData[0xE0];

    /* 0x07864 */ struct HBlankEffect hBlankEffect;
    /* 0x08C6C */ struct HBlankEffect hBlankEffect_backup;

    /* 0x0A074 */ u8 paused:1; /* freezes entity updates and palette fades */
    /* 0x0A074 */ u8 startPauseEnabled:1;
    /* 0x0A074 */ u8 bgControlMinWriteLevel:2;
    /* 0x0A074 */ u8 unusedWriteLevel:2; /* always set with bgControlMinWriteLevel, never read */
    /* 0x0A074 */ u8 collisionBlocksActive:2; /* collisionBlocks in use */

    /* 0x0A075 */ u8 konamiCodeInputsCounter;
    /* 0x0A076 */ u8 pad_A076[0xA078 - 0xA076];
    /* 0x0A078 */ struct BgLayerState bgInfo[4];
    /* 0x0A0E8 */ struct BgMapMetadata *savedBgMetadata;
    /* 0x0A0EC */ u16 savedBgScrollX;
    /* 0x0A0EE */ u16 savedBgScrollY;
    /* 0x0A0F0 */ u8 pad_A0F0[0xA108 - 0xA0F0];
    /* 0x0A108 */ u16 bg1TileCache[1]; // Length?
    /* 0x0A10A */ u8 pad_A10A[0xC0EC - 0xA10A];
    /* 0x0C0EC */ u16 sharedBgTileCache[1]; // Length?
    /* 0x0C0EE */ u8 pad_C0EE[0xE0D0 - 0xC0EE];
    /* 0x0E0D0 */ u8 bg1CollisionCache[1]; // Length?
    /* 0x0E0D2 */ u8 pad_E0D2[0xF0C0 - 0xE0D1];
    /* 0x0F0C0 */ u32 collisionBlocks[0x380]; /* bit per 16x16 room cell: forced solid (Collision_SetBlock) */
    /* 0x0FEC0 */ s8 screenShakeOffsetX;
    /* 0x0FEC1 */ s8 screenShakeOffsetY;
    /* 0x0FEC2 */ u8 pad_FEC2[0xFEC4 - 0xFEC2];
    /* 0x0FEC4 */ struct ObjGfxEntry objGfx[0x10];
    /* 0x10004 */ s32 objGfxSaved; /* nonzero while ObjGfx_SaveState holds a backup */
    /* 0x10008 */ u8 objGfxBackup[0x10150 - 0x10008]; /* objGfx copy (0x140 used) */
    /* 0x10150 */ struct EwramData_unk19404 *objGfxSetBackup;
    /* 0x10154 */ u8 objGfxSetListBackup[0x10170 - 0x10154];
    /* 0x10170 */ struct SpriteSlot spriteSlots[0xF0];

    /* 0x11070 */ s16 objGfxFreeHead;
    /* 0x11072 */ s16 objGfxUsedHead;
    /* 0x11074 */ s16 spriteFreeHead;
    /* 0x11076 */ s16 spriteUsedHead;
    /* 0x11078 */ u8 objGfxListBackup[0x11080 - 0x11078];
    /* 0x11080 */ struct ObjAffineBackup objAffineBackup;
    /* 0x11588 */ u16 singleSpriteOam[4];
    /* 0x11590 */ s32 singleSpriteX;
    /* 0x11594 */ s32 singleSpriteY;
    /* 0x11598 */ s32 *textTileBuffer;
    /* 0x1159C */ u16 unk_1159C;
    /* 0x1159E */ u16 unk_1159E;
    /* 0x115A0 */ u16 text12Column;
    /* 0x115A2 */ u16 text12Row;
    /* 0x115A4 */ u16 text12ColumnPixelOffset;
    /* 0x115A6 */ u16 text12PixelX;
    /* 0x115A8 */ u16 text12PixelY;
    /* 0x115AA */ u16 text12RegionLeft;
    /* 0x115AC */ u16 text12RegionTop;
    /* 0x115AE */ u16 text12LastColumn;
    /* 0x115B0 */ u16 text12LastRow;
    /* 0x115B2 */ u16 text8Column;
    /* 0x115B4 */ u16 text8Row;
    /* 0x115B6 */ u16 text8ColumnPixelOffset;
    /* 0x115B8 */ u16 text8PixelX;
    /* 0x115BA */ u16 text8PixelY;
    /* 0x115BC */ u16 text8RegionLeft;
    /* 0x115BE */ u16 text8RegionTop;
    /* 0x115C0 */ u16 text8LastColumn;
    /* 0x115C2 */ u16 text8LastRow;
    /* 0x115C4 */ u16 textForegroundColor;
    /* 0x115C6 */ u16 textBackgroundColor;
    /* 0x115C8 */ u8 itemIconIds[8];
    /* 0x115D0 */ u8 savedItemIconIds[8];
    /* 0x115D8 */ u8 itemIconTiles[0x400]; // Struct or array?
    /* 0x119D8 */ u8 savedItemIconTiles[0x400]; // Struct or array?
    /* 0x11DD8 */ u8 itemIconGfxEntry;
    /* 0x11DD9 */ u8 itemIconCacheFlags;
    /* 0x11DDA */ u8 pad_11DDA[0x11DDC - 0x11DDA];

    /* 0x11DDC */ struct PaletteBuffer objPalette;
    /* 0x1261C */ struct PaletteBuffer bgPalette;

    /* 0x12E5C */ struct PaletteAnimState paletteAnim;

    /* 0x12FE0 */ s8 paletteAnimSkipFrame:1; /* PaletteAnim_Update skips one frame */
    /* 0x12FE1 */ s8 unk_12FE0_1:1;
    /* 0x12FE0 */ s8 softResetSkipOnce:1; // Length?
    /* 0x12FE0 */ s8 unk_12FE0_3:5; // Real/Length?
    /* 0x12FE1 */ u8 pad_12FE1[0x12FE4 - 0x12FE1];
    /* 0x12FE4 */ u32 paletteInvert; /* 16.16 strength of Palette_ApplyInvert; 1 = turn off */
    /* 0x12FE8 */ u32 paletteInvertSaved; /* Palette_SuspendInvert backup */
    /* 0x12FEC */ u16 objTileFlags; /* OBJ_TILES_* */
    /* 0x12FEE */ u8 pad_12FEE[0x12FF0 - 0x12FEE];
    /* 0x12FF0 */ struct ObjTileSlot objTileSlots[0x10];
    /* 0x13070 */ s16 objTileSlotCount; /* usable slots: 16, 8 or 4 */
    /* 0x13072 */ u8 pad_13072[0x13074 - 0x13072];
    /* 0x13074 */ u16 hitRectLeft;
    /* 0x13076 */ u16 hitRectTop;
    /* 0x13078 */ u16 hitRectRight;
    /* 0x1307A */ u16 hitRectBottom;
    /* 0x1307C */ u8 hitTimerOverride[4]; // set per index by Hit_SetTimerOverride; [1] read by Hit_GetTimerOverride1
    /* 0x1307E */ 
    /* 0x13080 */ struct WhipChainNode whipChainNodes[1];
    /* 0x13090 */ u8 pad_13090[0x13110 - 0x13090];

    /* 0x13110 */ struct PlayerState playerState;

    /* 0x1325C */ struct PlayerStatus playerStatus;

    /* 0x133EC */ u8 pad_133EC[0x133EE - 0x133EC];
    /* 0x133EE */ s16 unk_133EE;
    /* 0x133F0 */ s16 unk_133F0;
    /* 0x133F2 */ u8 unk_133F2; // what's going on here?
    /* 0x133F3 */ u8 unk_133F3;
    /* 0x133F4 */ u8 sharedScratchBuffer[0x1000];
    /* 0x143F4 */ u8 entityScratchBuffer[0x800]; // shared buffer; entity allocation arena (Room_ResetTransientState)
    /* 0x14BF4 */ u8 entityHeapBackup[0x800]; // backup of the 0x800 bytes from entityScratchBuffer (Heap_SaveAndReset)
    /* 0x153F4 */ u8 gfxDecompBuffer[0x153F8 - 0x153F4]; /* LZ77 output: 4-byte resource header ... */
    /* 0x153F8 */ u8 gfxDecompTiles[1]; /* ... followed by the tile data */
    /* 0x153F9 */ u8 pad_153F9[0x19404 - 0x153F9];

    /* 0x19404 */ struct EwramData_unk19404 objGfxFrameBuffers[6]; // TODO: figure out struct
    /* 0x25464 */ struct EwramData_unk19404 *objGfxSet; /* frame set currently decompressed (kind 2) */
    /* 0x25468 */ u8 objGfxSetList; /* type-2-style header + objGfxSetFrames form a frame list */
    /* 0x25469 */ u8 objGfxSetListHeader[0x2546C - 0x25469];
    /* 0x2546C */ void *objGfxSetFrames[6]; /* &objGfxFrameBuffers[i] */

    /* 0x25484 */ struct SoulTradePacket soulTradePackets[2];
    /* 0x254A4 */ u8 pad_254A4[0x254C4 - 0x254A4];
    /* 0x254C4 */ u16 trackedSoundIds[4];
    /* 0x254CC */ u16 musicOverrideSong;
    /* 0x254CE */ u16 specialRoomSong;
    /* 0x254D0 */ struct DebugMenuWork debugMenuWork;
    /* 0x254E0 */ u8 pad_254E0[0x25550 - 0x254E0];
    /* 0x25550 */ u8 debug4531State;
    /* 0x25551 */ u8 debug4531SubStage;
    /* 0x25552 */ u8 debug4531Flags;
    /* 0x25553 */ s8 pauseMenuPage; // highlighted row 0..6 of the list drawn by PauseMenu_DrawPageList
}; /* size = 0x25554 */

extern struct EwramData *gEwramData;

#endif /* EWRAM_STRUCTS_H */
