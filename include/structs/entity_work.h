#ifndef STRUCTS_ENTITY_WORK_H
#define STRUCTS_ENTITY_WORK_H

#include "types.h"

struct EwramData_EntityData;

/*
 * Per-entity-type views of the entity work area 0x4F4..0x50F
 * (EwramData_EntityData.ext, union EntityWork in structs/ewram.h).
 * Each structure covers the whole area from 0x4F4; offsets in the comments
 * are entity offsets as in ewram.h. Bytes 0x4F4 / 0x4F5 hold ENTITY_INDEX /
 * ENTITY_PARENT_INDEX and 0x4F8 ENTITY_OWNER for most entities (raw view);
 * the structures leave them as padding unless the type uses them otherwise.
 * Member types keep the width and signedness of the original accesses.
 */

/* Giant Ghost body (GiantGhost_*): 16.16 speed of the back-off swoop, the
 * room position of the player stored for the dive, a byte cleared by
 * GiantGhost_ResetAnimation and never read, and the hit-player flag (set by
 * the contact handler, cleared when the attack ends).
 */
struct GiantGhostWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 speed;
    /* 0x4FC */ u32 diveX;
    /* 0x500 */ s32 diveY;
    /* 0x504 */ u8 unusedFlag;
    /* 0x505 */ u8 hitPlayer;
};

/* Giant Ghost hands (GiantGhostHandA/B_Update, created in
 * EnemyGiantGhostCreate): the room point they turn towards (swaying by one
 * pixel per frame), hand index, sway direction and sway frame (flips after
 * 60 frames).
 */
struct GiantGhostHandWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 pointX;
    /* 0x500 */ s32 pointY;
    /* 0x504 */ u8 index;
    /* 0x505 */ u8 pad_505[0x506 - 0x505];
    /* 0x506 */ s8 swayDir;
    /* 0x507 */ u8 swayFrame;
};

/* Giant Ghost aura (GiantGhostAura_Update): pulse phase. */
struct GiantGhostAuraWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 phase;
};

/* Dead Crusader: guarding (crouched behind the shield after the player
 * attacked; DeadCrusader_OnHit then takes no damage). Cleared every frame by
 * EnemyDeadCrusaderUpdate and set again by DeadCrusader_UpdateWalk.
 */
struct DeadCrusaderWork {
    /* 0x4F4 */ u8 pad_4F4[0x505 - 0x4F4];
    /* 0x505 */ u8 guarding;
};

/* Imp / Tiny Devil (code_080C47F8.c, code_080C5200.c; the Tiny Devil flight
 * slots are in ext.tinyDevil): slot index of the linked entity -
 * the imp keeps its grab indicator, a death piece keeps the imp's slot.
 * Imp flight: frames until the next attack, braking bits (1 = braking
 * horizontally, 2 = vertically), bob phase of the hovering, the two "in
 * position" flags (x / y) that start the attack when both are set, and the
 * frame counter of the grab.
 */
struct ImpWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ s16 linkIndex;
    /* 0x4F8 */ u8 pad_4F8[0x4FA - 0x4F8];
    /* 0x4FA */ s16 attackTimer;
    /* 0x4FC */ s16 braking;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s16 bobPhase;
    /* 0x502 */ u8 pad_502[0x504 - 0x502];
    /* 0x504 */ u8 inPositionX;
    /* 0x505 */ u8 inPositionY;
    /* 0x506 */ s8 grabTimer;
};

/* Peeping Eye: its pupil, eyeball and chain links keep the eye (the other
 * slots are TU-local PEEPING_EYE_* names in code_08087DC0.c).
 */
struct PeepingEyeWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Tsuchinoko: HP at creation (Tsuchinoko_UpdateJump jumps more often below
 * half of it and always below a quarter).
 */
struct TsuchinokoWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 maxHp;
};

/* Werewolf: set by Werewolf_DamagePlayerOnContact when the dash (state 1,
 * sub-states 1..2) hit the player; the dash then stops (state 3).
 */
struct WerewolfWork {
    /* 0x4F4 */ s32 dashHit;
};

/* Dead Warrior (DeadWarrior_*): 16.16 charge walk speed (accelerates to
 * 0x20000, brakes by 0x1000) and the frame counter of the charge (stops
 * after 100; Create starts it at 50).
 */
struct DeadWarriorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 speed;
    /* 0x500 */ u8 chargeFrame;
};

/* Debug mode entities: the moving debug platform (DebugPlatform_*) sets its
 * carry flag when its floor correction catches the player
 * (DebugPlatform_GetCarryVelocity then moves the player with it and clears
 * it); the test enemy (DebugTestEnemy_Update) circles with this angle.
 */
struct DebugWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 testEnemyAngle;
    /* 0x4FC */ s8 platformCarrying;
};

/* Fleaman (EnemyFleaman*, Fleaman_*): airborne (1 when both spawn params are
 * 0xFF: starts in the air instead of snapping to the ground) and the index
 * of the animation it last started (0 / 2 / 3).
 */
struct FleamanWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 airborne;
    /* 0x4F9 */ u8 anim;
};

/* Mudman shot (MudmanShot_DamagePlayerOnContact, MudmanShot_Update): after a
 * hit it sticks to the entity it hit (ENTITY_OWNER) and keeps that entity's
 * 16.16 room position of the previous frame (the shot moves along with it).
 */
struct MudmanShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 ownerX;
    /* 0x500 */ s32 ownerY;
};

/* Persephone: contact hit (set on the owner by
 * Persephone_DamagePlayerOnContact, from the body or an attack box; a jump
 * attack that hit lands at once); attack box (Persephone_SpawnAttackBox):
 * frames it stays.
 */
struct PersephoneWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 boxTime;
    /* 0x504 */ u8 contactHit;
};

/* Push block (PushBlock_Update, object 01 group): flags - bit 0 / 1 pushed
 * from the left / right this frame (set by the group's wall callbacks), bit
 * 2 carrying a block, bit 3 falling, bit 4 scraping sound playing. Group
 * (PushBlockGroup_*): entity slots of its blocks (byte array).
 */
struct PushBlockWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 groupSlots[4];
    /* 0x4FC */ u32 flags;
};

/* Needles: bob phase of the floating variant (spawn parameter 0 nonzero;
 * random start, +0x400 per frame).
 */
struct NeedlesWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 bobPhase;
};

/* Death (Needles_SpawnDeathParts / NeedlesPart_Update /
 * Needles_UpdateDeath): live death parts; each part decrements it when it
 * ends.
 */
struct NeedlesDeathWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 parts;
};

/* Doors (objects 0x02 / 0x04 / 0x06, Door_Draw / Door_SetupGraphics): - open
 * height: 0 = closed .. 0x30 = fully raised (Door_Draw draws the door that
 * many pixels higher; stepped by 1 or 2 per frame); - on right: the door is
 * in the right half of the screen (x > 0x78), the direction the player is
 * walked through it; - palette slot.
 */
struct DoorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 openHeight;
    /* 0x4FC */ u8 pad_4FC[0x4FE - 0x4FC];
    /* 0x4FE */ u8 onRight;
    /* 0x4FF */ u8 palette;
};

/* Kicker Skeleton (KickerSkeleton_*): - flags: bit 0 while kicking (stronger
 * contact damage with knockback); - speed: 16.16 walk speed of the kick
 * dash; - hit side: +1 / -1, side the killing hit came from (scatters
 * fragments).
 */
struct KickerSkeletonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 flags;
    /* 0x4FC */ u32 speed;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ s32 hitSide;
};

/* Mimic (EnemyMimic*, Mimic_*): per-frame flags (bit 0 = no contact damage,
 * bit 1 = show the held frame of sMimicAnimation) and the held frame. Its
 * trigger (MimicTrigger_*) links back to it through trigger owner.
 */
struct MimicWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 flags;
    /* 0x4FD */ u8 heldFrame;
};

/* Mimic (EnemyMimic*, Mimic_*): per-frame flags (bit 0 = no contact damage,
 * bit 1 = show the held frame of sMimicAnimation) and the held frame. Its
 * trigger (MimicTrigger_*) links back to it through trigger owner.
 */
struct MimicTriggerWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Sky Fish (EnemySkyFishCreate, SkyFish_UpdateFly): darting flag (1 while it
 * flies its fast pass, 0 when stopped by a time stop), start side (0 = left,
 * 1 = right edge) and the random delay before it appears.
 */
struct SkyFishWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 darting;
    /* 0x4FD */ u8 fromRight;
    /* 0x4FE */ s16 delay;
};

/* Work slots of yellow soul entities (created from YellowSoulInfo.updateFunc
 * by YellowSoul_CreateEquippedEntity / YellowSoul_Restart; one at a time in
 * playerState.activeYellowSoulEntity): - owner: the player; - param / value:
 * YellowSoulInfo.effectParam / effectValue (which stat or flag the soul
 * changes and by how much).
 */
struct YellowSoulWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *owner;
    /* 0x4FC */ u8 pad_4FC[0x4FE - 0x4FC];
    /* 0x4FE */ s16 param;
    /* 0x500 */ s32 value;
};

/* Ghost spawner (EnemyGhostUpdate state 3): number of live ghosts (at most
 * spawnParam0; a ghost decrements its owner's count when it is deleted) and
 * the frames since the last spawn (spawnParam1 minimum, then 1 in 32).
 */
struct GhostSpawnerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 childCount;
    /* 0x4FE */ s16 spawnDelay;
};

/* Float (Ghost_UpdateFloat): random vertical offset (0..-16 px, 16.16) it
 * keeps from the target point while near (TargetOffsetY).
 */
struct GhostWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s32 aimOffsetY;
};

/* Mandragora (Mandragora_*, MandragoraPart_*): the body and each stem part
 * keep the entity index of the next linked part (towards the tip); the body
 * counts its live death parts (MandragoraDeathPart_Update decrements it).
 * Flag byte set by its parts (TU aliases FLAGS): 2, 8, 0x10, 0x20 - bits 3 /
 * 4 end the scream (Mandragora_UpdateScream).
 */
struct MandragoraWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 linkIndex;
    /* 0x4F7 */ u8 pad_4F7[0x504 - 0x4F7];
    /* 0x504 */ u8 flags;
};

/* Mandragora (Mandragora_*, MandragoraPart_*): the body and each stem part
 * keep the entity index of the next linked part (towards the tip); the body
 * counts its live death parts (MandragoraDeathPart_Update decrements it).
 */
struct MandragoraDeathWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 parts;
};

/* Floating background particle (BgParticle_SpawnAll, BgParticle_Update): -
 * kind: 0..3 (size class; animation index and the speed shift; kind 3 uses
 * the second angle); - owner index: entity index of the spawning entity; -
 * angle x / y: phases of the sine drift.
 */
struct BgParticleWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angleX;
    /* 0x4FC */ u32 angleY;
    /* 0x500 */ u8 kind;
    /* 0x501 */ s8 ownerIndex;
};

/* Ectoplasm (EnemyEctoplasm*, Ectoplasm_*): 16.16 room point it floats to
 * (the spawn point at first, then random points near the target), bob phase
 * of the floating, and the retarget toggle (alternates near / far points).
 */
struct EctoplasmWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 pointX;
    /* 0x4FC */ u32 pointY;
    /* 0x500 */ s32 bobPhase;
    /* 0x504 */ u8 retarget;
};

/* Event scene 9 controller: the two helper entities created from the object
 * table (kinds 5 and 1; actor 5 has an affine sprite that is squashed by the
 * scale x / scale y slots and then hidden), as in event scene 3 the actors
 * are named after their object table kind.
 */
struct Event09Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *actor5;
    /* 0x4FC */ struct EwramData_EntityData *actor1;
    /* 0x500 */ s32 scaleX;
    /* 0x504 */ s32 scaleY;
};

/* Head stack (HeadStack_Create / HeadStack_Update): entity indices of its
 * segments, bottom first (spawnParam0 = height 1..8; 0 = destroyed), and the
 * on-screen flag of the keeper. Segments (HeadStackSegment_*): position in
 * the column (0 = bottom; its parity picks the attack) and the variant index
 * into sUnk_081193DC (graphics, debris, attacks), which their effects and
 * projectiles read from the segment or keep themselves.
 */
struct HeadStackWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F5 - 0x4F4];
    /* 0x4F5 */ u8 level;
    /* 0x4F6 */ u8 type;
    /* 0x4F7 */ u8 pad_4F7[0x4F9 - 0x4F7];
    /* 0x4F9 */ u8 onScreen;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ u8 slots[4];
};

/* Skull Archer (SkullArcher_*): step 0..4 through the shot patterns (index /
 * 2 into sUnk_08528614), the current pattern pointer, and the side (+1 / -1)
 * the killing hit came from (scatters the death pieces). Arrow
 * (SkullArcherArrow_*): the archer that shot it.
 */
struct SkullArcherWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 patternStep;
    /* 0x4F9 */ u8 pad_4F9[0x500 - 0x4F9];
    /* 0x500 */ s32 pattern;
    /* 0x504 */ s32 hitSide;
};

/* Skull Archer (SkullArcher_*): step 0..4 through the shot patterns (index /
 * 2 into sUnk_08528614), the current pattern pointer, and the side (+1 / -1)
 * the killing hit came from (scatters the death pieces). Arrow
 * (SkullArcherArrow_*): the archer that shot it.
 */
struct SkullArcherArrowWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *owner;
};

/* Skull Millione family (SkullMillioneFamily_*): variant (index into
 * sSkullMillioneFamilyVariants), bob frame of the wait state and the 16.16
 * approach speed (accelerates to the variant's approachSpeed). Part
 * (SkullMillioneFamilyPart_Update): affine angle.
 */
struct SkullMillioneWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 variant;
    /* 0x4F9 */ u8 pad_4F9[0x4FC - 0x4F9];
    /* 0x4FC */ s16 bobFrame;
    /* 0x4FE */ u8 pad_4FE[0x504 - 0x4FE];
    /* 0x504 */ s32 speed;
};

/* Skull Millione family (SkullMillioneFamily_*): variant (index into
 * sSkullMillioneFamilyVariants), bob frame of the wait state and the 16.16
 * approach speed (accelerates to the variant's approachSpeed). Part
 * (SkullMillioneFamilyPart_Update): affine angle.
 */
struct SkullMillionePartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FE - 0x4F4];
    /* 0x4FE */ s16 angle;
};

/* Harpy (Harpy_UpdateAttack): room point of the dive (0x18 px above the
 * target) and the y it started from. Feathers (Harpy_ThrowFeather /
 * _ThrowWavyFeather, HarpyWavyFeather_Update): flight angle.
 */
struct HarpyWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 diveX;
    /* 0x4FC */ u32 diveY;
    /* 0x500 */ s32 startY;
};

/* Harpy (Harpy_UpdateAttack): room point of the dive (0x18 px above the
 * target) and the y it started from. Feathers (Harpy_ThrowFeather /
 * _ThrowWavyFeather, HarpyWavyFeather_Update): flight angle.
 */
struct HarpyFeatherWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angle;
};

/* Lilith (EnemyLilith*, Lilith_*): - walk frame: frames of the current walk
 * (re-decides after 60); - dodge checked: the dodge chance for the current
 * player attack was already rolled (cleared when the player stops
 * attacking); - hidden: 1 until it appears (no contact damage while hidden).
 */
struct LilithWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 walkFrame;
    /* 0x4FD */ u8 dodgeChecked;
    /* 0x4FE */ u8 pad_4FE[0x4FF - 0x4FE];
    /* 0x4FF */ u8 hidden;
};

/* Lunge hit: set by Lilith_DamagePlayerOnContact; a lunge that hit the
 * player holds its animation and chains a second lunge
 * (Lilith_UpdateAttack).
 */
struct LilithLungeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FE - 0x4F4];
    /* 0x4FE */ u8 hit;
};

/* Ripper (EnemyRipperCreate, Ripper_*; same template as the Fleaman):
 * airborne (1 when both spawn params are 0xFF: starts in the air instead of
 * snapping to the ground), the index of the animation it last started, and
 * the thrown flag (a knife was thrown during the current jump).
 */
struct RipperWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 airborne;
    /* 0x4F9 */ u8 anim;
    /* 0x4FA */ u8 pad_4FA[0x504 - 0x4FA];
    /* 0x504 */ u8 thrown;
};

/* Projectile (Ripper_CreateProjectile, RipperProjectile_Update): its Ripper. */
struct RipperProjectileWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ struct EwramData_EntityData *owner;
};

/* Waiter Skeleton (WaiterSkeleton_*): flags - bit 0 the tray was thrown, bit
 * 1 the tray still carries its item, bit 2 set from the walk's third
 * animation step, bit 3 a thrown tray hit the player.
 */
struct WaiterSkeletonWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s32 flags;
};

/* Trays (WaiterSkeletonTray_Update, WaiterSkeletonThrownTray_*): owner,
 * animation script of the thrown object (17 = item, 9 = tray) and the affine
 * angle of the spinning thrown tray.
 */
struct WaiterTrayWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 anim;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ struct EwramData_EntityData *owner;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ s32 angle;
};

/* Ghost Dancer (EnemyGhostDancerCreate, GhostDancer_*; the TU aliases PHASE
 * / MODE / TIMER read the same bytes): sine phase of the dance (+128 per
 * frame), number of completed cycles, the side it sways to (toggles), dance
 * mode (advanced every half cycle) and the mode timer.
 */
struct GhostDancerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 phase;
    /* 0x4FA */ u8 cycles;
    /* 0x4FB */ u8 side;
    /* 0x4FC */ s8 mode;
    /* 0x4FD */ u8 timer;
};

/* Medusa Head (EnemyMedusaHead*, MedusaHead_*): - wave up: 1 while the
 * vertical acceleration is reversed; - wave frame / wave length: frames of
 * the current wave half and its length (random), wave up flips when it is
 * reached; - a spawner keeps its live heads in child count, each head links
 * back to it through spawner.
 * Medusa Head (EnemyMedusaHead*, MedusaHead_*): - wave up: 1 while the
 * vertical acceleration is reversed; - wave frame / wave length: frames of
 * the current wave half and its length (random), wave up flips when it is
 * reached; - a spawner keeps its live heads in child count, each head links
 * back to it through spawner.
 */
struct MedusaHeadWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 childCount;
    /* 0x500 */ struct EwramData_EntityData *spawner;
    /* 0x504 */ u8 waveUp;
    /* 0x505 */ u8 waveFrame;
    /* 0x506 */ s8 waveLength;
};

/* Quezlcoatl (EnemyQuezlcoatl*, Quezlcoatl_*, QuezlcoatlSegment_Update): -
 * history: heap buffer (60 bytes) of past head positions the segments follow
 * (owner's buffer in the segments); - bounce count: water bounces since the
 * last turn (re-faces the target on even counts); - camera x / y: camera
 * position of the previous frame (scroll correction, head and segments).
 */
struct QuezlcoatlWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 bounceCount;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ s16 cameraX;
    /* 0x4FE */ s16 cameraY;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ s32 history;
};

/* Quezlcoatl (EnemyQuezlcoatl*, Quezlcoatl_*, QuezlcoatlSegment_Update): -
 * history: heap buffer (60 bytes) of past head positions the segments follow
 * (owner's buffer in the segments); - bounce count: water bounces since the
 * last turn (re-faces the target on even counts); - camera x / y: camera
 * position of the previous frame (scroll correction, head and segments).
 */
struct QuezlcoatlSegmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ u8 index;
};

/* Evil Butcher projectile (EvilButcherProjectile_Update): affine angle, spin
 * step and the x offset (+-6 px) of its ground probe. Debris
 * (EvilButcherDebris_Update): affine angle and spin step.
 */
struct EvilButcherProjectileWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 spin;
    /* 0x500 */ s16 probeX;
};

/* Evil Butcher projectile (EvilButcherProjectile_Update): affine angle, spin
 * step and the x offset (+-6 px) of its ground probe. Debris
 * (EvilButcherDebris_Update): affine angle and spin step.
 */
struct EvilButcherDebrisWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 spin;
};

/* Gate (Gate_*, object 0): opening angle of the two wings (0 = closed,
 * 0x4000 = open, stepped by 0x400), the affine slots of the two wings
 * (Gate_Draw reads them as ext.gateDraw.affine[0] / [1]) and the palette slot.
 */
struct GateWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 openAngle;
    /* 0x4FC */ s8 affine0;
    /* 0x4FD */ u8 affine1;
    /* 0x4FE */ u8 pad_4FE[0x4FF - 0x4FE];
    /* 0x4FF */ u8 palette;
};

/* Both wing affine slots as a byte array (Gate_Draw). */
struct GateDrawWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u8 affine[4];
};

/* Killer Mantle (KillerMantle_*): - speed: 16.16 walk speed (hover sway
 * amplitude, random per sway; jump attack speed, braked to 0); - sway frame:
 * frame of the current hover sway (turns after 48); - flags: bit 0 set
 * during the jump attack (spreads the cape part).
 */
struct KillerMantleWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 speed;
    /* 0x4FC */ s16 swayFrame;
    /* 0x4FE */ s16 flags;
};

/* Cape part (KillerMantlePart_Update): affine angle and x scale. */
struct KillerMantlePartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FE - 0x4F4];
    /* 0x4FE */ s16 angle;
    /* 0x500 */ s32 scaleX;
};

/* Golem family (EnemyGolemCreate, Golem_*; joint slots in joint_entity.h): -
 * variant: index into the per-variant timing tables (from the enemy id,
 * 0..3); - attack phase: 0 during the swing, 1 when the punch lands (read by
 * the golem segments' contact handler).
 * Parts: heap block of its part records (struct GolemPart).
 */
struct GolemWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 attackPhase;
    /* 0x4FD */ u8 pad_4FD[0x500 - 0x4FD];
    /* 0x500 */ struct GolemPart *parts;
    /* 0x504 */ u8 pad_504[0x508 - 0x504];
    /* 0x508 */ s32 variant;
};

/* Body segments (GolemSegment_Update; part index / anchor / owner in
 * joint_entity.h): spin speed and affine angle of the fall after death.
 */
struct GolemSegmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 spin;
    /* 0x4FC */ u32 fallAngle;
};

/* Bestiary preview (Golem_UpdateBestiaryPreview): idle effects shown so far. */
struct GolemPreviewWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 effects;
};

/* Arachne work slots: - walk phase: 16.16 position in the leg cycle (wraps
 * at 0x240000; the legs pick their pose from its integer part), walk speed:
 * signed step added to it every frame (eased to +-0x10000); - gait: leg pose
 * mode (0 walk, 1 / 2 recoil), wait: frames to wait; - attack angle: aim of
 * the web shot (swept by 0x60 per frame); Legs keep their index (0..n) in
 * leg index.
 */
struct ArachneWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 attackAngle;
    /* 0x4FC */ u32 walkPhase;
    /* 0x500 */ s32 walkSpeed;
    /* 0x504 */ u8 legIndex;
    /* 0x505 */ u8 gait;
    /* 0x506 */ s8 wait;
};

/* Kyoma Demon (EnemyKyomaDemon*, KyomaDemon_*): - hidden: 1 while it lurks
 * behind the scenery (no contact damage); - lurk timer: frames until it next
 * appears (random 64..127); - home x / y: room pixel position of the spawn
 * point; - room x / y: 16.16 room position of the previous frame (Lurk
 * compares it to detect that it stopped).
 */
struct KyomaDemonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 homeX;
    /* 0x4FA */ s16 homeY;
    /* 0x4FC */ s8 hidden;
    /* 0x4FD */ u8 pad_4FD[0x4FE - 0x4FD];
    /* 0x4FE */ s16 lurkTimer;
    /* 0x500 */ s32 roomX;
    /* 0x504 */ s32 roomY;
};

/* Nightmare (running horse): - run speed: 16.16 speed passed to Entity_Walk
 * (accelerates to 0x28000, brakes by 0x1000); - run timer: frames of the
 * current run (turns after 0x50); - effect index 0 / 1: entity indices of
 * its two effect entities (ended when it dies); - room x / y: room position
 * saved for Nightmare_Draw.
 */
struct NightmareWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 runSpeed;
    /* 0x500 */ u8 runTimer;
    /* 0x501 */ u8 pad_501[0x504 - 0x501];
    /* 0x504 */ s16 roomX;
    /* 0x506 */ s16 roomY;
};

/* Nightmare (running horse): - run speed: 16.16 speed passed to Entity_Walk
 * (accelerates to 0x28000, brakes by 0x1000); - run timer: frames of the
 * current run (turns after 0x50); - effect index 0 / 1: entity indices of
 * its two effect entities (ended when it dies); - room x / y: room position
 * saved for Nightmare_Draw.
 */
struct NightmareEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 index0;
    /* 0x4F9 */ u8 index1;
};

/* Work slots of Buer and Bael, the rolling wheel enemies (Bael's code is a
 * copy of Buer's with its own data): - speed: signed 16.16 rolling speed
 * passed to Entity_Walk (sign = direction, eased and bounced off walls); -
 * angle: affine rotation, advanced by speed / 64 while rolling; - direction:
 * turn direction while charging; Their charge effect entities keep the
 * animation index to play.
 */
struct WheelWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 effectAnim;
    /* 0x4F7 */ u8 direction;
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ u32 speed;
};

/* Death (Buer_UpdateDeath / Bael_UpdateDeath - TU aliases SHAKE_X /
 * SHAKE_Y): shake offset of the head part, bytes 0 / 1 of the angle slot.
 */
struct WheelDeathWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 shake[4];
};

/* Bael charge (Bael_UpdateCharge): charges rolled so far, kept on the entity
 * at ENTITY_PARENT_INDEX; after 1..5 of them the charge stops.
 */
struct BaelWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 chargeCount;
};

/* Work slots shared by the multi-part ("joint") enemies Golem, GreatArmor,
 * Devil, CreakingSkull, Stolas and BigGolem. The owner keeps its parts in
 * unk_500 (GolemPart/EntityJointPart arrays) and selects a pose: the index
 * into its s<Owner>PartPoses / s<Owner>PartAngles tables (read by
 * <Owner>_UpdatePart / _InterpolateJoint and the create functions). Each
 * part entity stores the index of the owner part it follows and the part it
 * turns towards; the owner's hurt mask has one bit per part whose box
 * damages the player (part updates test (mask >> partIndex) & 1).
 * Parts: heap block of the joint part records (Stolas 6, Creaking Skull 13).
 */
struct JointWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EntityJointPart *parts;
    /* 0x504 */ u8 pose;
    /* 0x505 */ u8 pad_505[0x506 - 0x505];
    /* 0x506 */ s16 hurtMask;
};

/* Work slots shared by the multi-part ("joint") enemies Golem, GreatArmor,
 * Devil, CreakingSkull, Stolas and BigGolem. The owner keeps its parts in
 * unk_500 (GolemPart/EntityJointPart arrays) and selects a pose: the index
 * into its s<Owner>PartPoses / s<Owner>PartAngles tables (read by
 * <Owner>_UpdatePart / _InterpolateJoint and the create functions). Each
 * part entity stores the index of the owner part it follows and the part it
 * turns towards; the owner's hurt mask has one bit per part whose box
 * damages the player (part updates test (mask >> partIndex) & 1).
 * Part owner: every part entity points back to its owner (set by the owner's
 * create function).
 */
struct JointPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *owner;
    /* 0x504 */ u8 index;
    /* 0x505 */ u8 anchor;
};

/* Attack phase: reset to 0 when a swing / attack step starts (Great Armor,
 * Creaking Skull, Big Golem); only the Golem also sets it to 1 when the
 * punch lands and reads it in its segments (ext.golem.attackPhase, same
 * byte).
 */
struct JointAttackWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 phase;
};

/* Common work slots of blue soul entities (created by
 * Soma_HandleBlueSoulInput from BlueSoulInfo.updateFunc) and of their helper
 * entities (created with BlueSoul_CreateEntity): - owner: the entity that
 * created it (the player for the soul itself); - slot: index in
 * playerState.activeBlueSoulEntities (0 for the soul), cleared when the
 * entity ends; - id: the equipped blue soul; param / value:
 * BlueSoulInfo.effectParam / effectValue (variant or strength, e.g. Devil /
 * Manticore / Curly 0..2); - MP interval / counter: one MP is paid every 60
 * / mpPerSecond frames, counted by BlueSoul_ShouldEnd /
 * BlueSoul_CheckEndCondition.
 */
struct BlueSoulWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *owner;
    /* 0x4FC */ s16 id;
    /* 0x4FE */ s16 param;
    /* 0x500 */ s32 value;
    /* 0x504 */ u8 mpInterval;
    /* 0x505 */ u8 pad_505[0x506 - 0x505];
    /* 0x506 */ s16 mpCounter;
    /* 0x508 */ s16 slot;
};

/* Boss recovery pickup (BossRecoveryPickup_Update - TU aliases PHASE /
 * LANDED / SCALE): bob phase, landed flag and the affine scale derived from
 * the phase. Its particles (BossRecoveryPickupParticle_Update - TU aliases
 * ANGLE / STEP / RADIUS / SPEED): angle, angle step, 16.16 radius (they
 * spiral in until it is <= 0) and the accelerating step growth.
 */
struct BossPickupWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 phase;
    /* 0x4FA */ s16 landed;
    /* 0x4FC */ u32 scale;
};

/* Boss recovery pickup (BossRecoveryPickup_Update - TU aliases PHASE /
 * LANDED / SCALE): bob phase, landed flag and the affine scale derived from
 * the phase. Its particles (BossRecoveryPickupParticle_Update - TU aliases
 * ANGLE / STEP / RADIUS / SPEED): angle, angle step, 16.16 radius (they
 * spiral in until it is <= 0) and the accelerating step growth.
 */
struct BossPickupParticleWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 step;
    /* 0x504 */ s32 radius;
    /* 0x508 */ s32 speed;
};

/* Objects that move with BG layer 1 (objects 0x29, 0x2A, 0x32, 0x34 and the
 * 0x34 parts) keep their position in layer coordinates (screen position +
 * bgInfo[1] scroll) and derive posX / posY from it every frame; the screen
 * position is also saved for their draw functions. Shuttle platforms (0x29
 * vertical, 0x2A horizontal; Object32Create fills the same fields) move
 * between the start and the end coordinate (spawnParam0 << 16); shuttleDir
 * toggles at each end (initially 1 when the start lies beyond the end).
 */
struct LayerObjectWork {
    /* 0x4F4 */ s32 x;
    /* 0x4F8 */ s32 y;
    /* 0x4FC */ u32 shuttleStart;
    /* 0x500 */ s32 shuttleEnd;
    /* 0x504 */ u8 shuttleDir;
    /* 0x505 */ u8 pad_505[0x508 - 0x505];
    /* 0x508 */ s32 drawX;
    /* 0x50C */ s32 drawY;
};

/* Minotaur (Minotaur_*): animation step copied each frame (read by the swing
 * trail), variant (0 = attacks without the distance check), flags (bit 0 =
 * the axe follows the hand, bit 1 = it still has its axe; both cleared when
 * hit to death).
 */
struct MinotaurWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 animStep;
    /* 0x4F9 */ u8 variant;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ u32 flags;
};

/* Axe (MinotaurAxe_Update; spin, spin speed and dropped flag are TU macros
 * there): its angle, read by MinotaurAxeHitBox_Update. Swing trail
 * (MinotaurSwingTrail_Update): affine angle.
 * Axe (MinotaurAxe_Update - TU aliases SPIN / SPIN_SPEED / DROPPED): affine
 * angle and spin speed of the falling axe and the dropped flag; it follows
 * the owner's ext.minotaur.animStep while held.
 */
struct MinotaurAxeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 spin;
    /* 0x4FE */ s16 angle;
    /* 0x500 */ s16 spinSpeed;
    /* 0x502 */ s16 dropped;
};

/* Axe (MinotaurAxe_Update; spin, spin speed and dropped flag are TU macros
 * there): its angle, read by MinotaurAxeHitBox_Update. Swing trail
 * (MinotaurSwingTrail_Update): affine angle.
 */
struct MinotaurTrailWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
};

/* Stolas (EnemyStolasUpdate; joint slots in the joint view): per-frame flags
 * (cleared every update; bit 0 = show the held frame instead of the
 * animation), the held frame, the draw offset in whole pixels (Stolas_Draw
 * adds it to the position; (0, -29) from EnemyStolasCreate), the beam length
 * of the beam attack (passed to Stolas_SpawnBeamSegment) and the free child
 * slots (3 minus its live children; the beam attack starts only while one is
 * free).
 */
struct StolasWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 drawOffsetX;
    /* 0x4FA */ s16 drawOffsetY;
    /* 0x4FC */ s8 flags;
    /* 0x4FD */ u8 heldFrame;
    /* 0x4FE */ u8 pad_4FE[0x508 - 0x4FE];
    /* 0x508 */ s16 beamLength;
    /* 0x50A */ s16 freeSlots;
};

/* Stolas effects (Stolas_SpawnCastEffect, beam segments and impacts):
 * Stolas, and the affine angle of the cast effect (turns to +-0x10000).
 */
struct StolasEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 castAngle;
    /* 0x4FC */ u8 pad_4FC[0x500 - 0x4FC];
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Worm (Worm_Init builds a root entity and a chain of segments): every
 * segment (and the root itself) points to the root; each segment follows the
 * previous one; segment index = position in the chain (0 = head end, 4 =
 * tail); crawl dir (0 / 1) and turn timer (frames until it picks a direction
 * again) of the root; the root's variant (0 = ground worm, else it
 * undulates); aggressive = set on the root when it starts chasing; wave
 * phase of the undulation per segment; entity slot of the next body segment
 * (Worm_Draw).
 */
struct WormWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *root;
    /* 0x4FC */ struct EwramData_EntityData *previous;
    /* 0x500 */ u8 aggressive;
    /* 0x501 */ s8 turnTimer;
    /* 0x502 */ u8 segmentIndex;
    /* 0x503 */ u8 crawlDir;
    /* 0x504 */ u8 nextIndex;
    /* 0x505 */ u8 variant;
    /* 0x506 */ s16 wavePhase;
};

/* Common work slots of red soul entities (Soma's bullet souls and Julius'
 * sub-weapons, created from RedSoulInfo.updateFunc) and of the helper
 * entities created with RedSoul_CreateEntity: owner = the entity that
 * created it (the player for a cast); slot = index in
 * playerState.activeRedSoulEntities, cleared when the entity ends; id = the
 * red soul that was cast (casts only); param = RedSoulInfo.param, the
 * variant of a shared update function or a duration (casts only); hit = set
 * to 1 by SoulAttack_MarkHit. A soul attack that sticks to the enemy it hit
 * (RedSoul_AnchorToHitTarget / _AnchorToHitTargetTimed) keeps its anchor
 * entity and counts the frames since (timed variant).
 */
struct RedSoulWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *owner;
    /* 0x4FC */ s8 hit;
    /* 0x4FD */ u8 pad_4FD[0x4FE - 0x4FD];
    /* 0x4FE */ s16 slot;
    /* 0x500 */ s16 id;
    /* 0x502 */ s16 param;
    /* 0x504 */ struct EwramData_EntityData *anchor;
    /* 0x508 */ u8 pad_508[0x50A - 0x508];
    /* 0x50A */ s16 anchorTimer;
};

/* Hit anchor of a soul attack (RedSoulAnchor_Update, owner = the attack):
 * the enemy it hit (the attack moves along with it) and the flag set when
 * that enemy's hp runs out.
 */
struct RedSoulAnchorWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ struct EwramData_EntityData *target;
    /* 0x508 */ u8 targetDead;
};

/* Killer Doll / Lightning Doll death (KillerDoll_*, LightningDoll_*): the
 * doll counts its live death parts (spawned by *_SpawnDeathParts, each part
 * decrements it when it ends).
 */
struct DollWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ u8 deathPartCount;
};

/* Doll death part: its index (row of s*DollPartOffsets) and the entity index
 * of the effect it spawned.
 */
struct DollPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 effectIndex;
    /* 0x4F7 */ u8 pad_4F7[0x501 - 0x4F7];
    /* 0x501 */ s8 index;
};

/* Killer Doll countdowns (count down every frame): until it may lunge at the
 * player, until its turn animation, until it sinks to the floor and rests.
 */
struct KillerDollWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 lungeTimer;
    /* 0x4FA */ s16 turnTimer;
    /* 0x4FC */ s16 restTimer;
};

/* Lightning Doll countdowns (count down every frame): until it may attack,
 * frames of the current move, until it rests.
 * Unused effect index: index of the player effect in its (one-pass) spawn
 * loop, never read.
 */
struct LightningDollWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 attackTimer;
    /* 0x4FA */ s16 moveTimer;
    /* 0x4FC */ s16 restTimer;
    /* 0x4FE */ u8 pad_4FE[0x501 - 0x4FE];
    /* 0x501 */ s8 unusedEffectIndex;
};

/* Belmont (boss) work slots: - attack: contact attack power from the
 * template, restored after moves; attack off: always 0, copied to attack to
 * disable contact damage during dodges, slides and reactions; - flags: state
 * bits (8 = ignore hit reactions, 0x10 = scene objects made); - flash:
 * frames of the hit flash / reaction; - hits: hits taken since the last
 * reaction (reacts after 3); - weapon index: entity index of his attached
 * weapon; - power step: counter of the powered phase.
 */
struct BelmontWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 flags;
    /* 0x4F9 */ u8 flash;
    /* 0x4FA */ u8 powerStep;
    /* 0x4FB */ u8 pad_4FB[0x500 - 0x4FB];
    /* 0x500 */ u8 attack;
    /* 0x501 */ s8 attackOff;
    /* 0x502 */ u8 pad_502[0x504 - 0x502];
    /* 0x504 */ u8 hits;
};

/* Belmont (boss) work slots: - attack: contact attack power from the
 * template, restored after moves; attack off: always 0, copied to attack to
 * disable contact damage during dodges, slides and reactions; - flags: state
 * bits (8 = ignore hit reactions, 0x10 = scene objects made); - flash:
 * frames of the hit flash / reaction; - hits: hits taken since the last
 * reaction (reacts after 3); - weapon index: entity index of his attached
 * weapon; - power step: counter of the powered phase.
 */
struct BelmontWeaponWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 index;
};

/* Scene object (Belmont_CreateSceneObject / _UpdateIntro /
 * _UpdateSceneBoundary): its entity index.
 */
struct BelmontSceneWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FB - 0x4F4];
    /* 0x4FB */ u8 objectIndex;
};

/* Power particles (Belmont_CreatePowerParticle,
 * BelmontPowerParticle_Update): step (set to the powered phase state by
 * Belmont_UpdatePoweredPhase).
 */
struct BelmontParticleWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 step;
};

/* Killer Fish (big fish, KillerFish_*): - stall timer / last x: room x of
 * the previous frame and the frames spent at it; after 0x3C stalled frames
 * the fish turns around; - effect timer: countdown to the next oscillating
 * swim effect (random 0x78..0xEF frames); - attack x / y: room pixel
 * position sampled as the attack target.
 */
struct KillerFishWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 stallTimer;
    /* 0x4FC */ s16 lastX;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s16 attackX;
    /* 0x502 */ s16 attackY;
};

/* Killer Fish (big fish, KillerFish_*): - stall timer / last x: room x of
 * the previous frame and the frames spent at it; after 0x3C stalled frames
 * the fish turns around; - effect timer: countdown to the next oscillating
 * swim effect (random 0x78..0xEF frames); - attack x / y: room pixel
 * position sampled as the attack target.
 */
struct KillerFishEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FE - 0x4F4];
    /* 0x4FE */ s16 timer;
};

/* Swarm fish (KillerFishSwarm_*): bite flag, set by the contact handler
 * (KillerFishSwarm_DamagePlayerOnContact) and ending the dart.
 * Swarm fish movement (code_08070218.c - TU aliases SCATTER / OWNER_LOST /
 * STEER_TIMER): scatter flag, set when the leader (ENTITY_CREATOR) is gone,
 * and the frames until the next steering change.
 */
struct KillerFishSwarmWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 steerTimer;
    /* 0x4FC */ s8 scatter;
    /* 0x4FD */ u8 ownerLost;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s32 bitPlayer;
};

/* Pause-menu page entity (the TU-local struct MenuPage views call the same
 * bytes count / cursor / top / item). MenuPage_Reset clears category and
 * these four bytes; MenuPage_SetMode stores the initial cursor / mode and
 * clears top. category: soul menu list category (SoulMenu_CountList /
 * DrawList); count: entries on the page; cursor: selected entry, wraps at
 * count; top: first visible entry; item: number of listed items (ItemMenu /
 * SoulMenu_CountList).
 */
struct MenuPageWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 category;
    /* 0x4F8 */ s8 count;
    /* 0x4F9 */ u8 cursor;
    /* 0x4FA */ u8 top;
    /* 0x4FB */ u8 item;
};

/* The four page bytes read as an array (cursor drawing, list counting): [0]
 * = count, [1] = cursor, [2] = top, [3] = item.
 */
struct MenuPageListWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 bytes[4];
};

/* Graphics slot of the menu sprites, loaded into the cursor entity (slot 18)
 * by MenuCursor_LoadGraphics and used for every menu sprite; the item icon
 * entities keep their icon graphics slot in the same byte. Button config
 * page: the action of each of its four rows (index into
 * sButtonConfigCursorFrames).
 */
struct MenuWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F5 - 0x4F4];
    /* 0x4F5 */ u8 gfxSlot;
    /* 0x4F6 */ u8 pad_4F6[0x4FC - 0x4F6];
    /* 0x4FC */ u8 pageButtons[4];
};

/* Page-title entity (slot 20; MenuPageTitle_Create / _Update / _Delete):
 * state (1 = place and start the zoom-in, 2 = zooming).
 */
struct MenuTitleWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ u8 state;
};

/* Ukoback (EnemyUkobackCreate, Ukoback_UpdateFlight): frames until the next
 * fireball (random 80..439) and the number of its live fireballs (at most 8;
 * a fireball decrements it when it goes out).
 */
struct UkobackWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FA - 0x4F4];
    /* 0x4FA */ s16 attackTimer;
    /* 0x4FC */ u8 pad_4FC[0x506 - 0x4FC];
    /* 0x506 */ s8 fireballCount;
};

/* Fireball (UkobackFireball_*): affine x / y scale, frame counter and the
 * Ukoback's entity index (its index halfword copied).
 */
struct UkobackFireballWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ s16 ownerIndex;
    /* 0x4F8 */ s32 scaleX;
    /* 0x4FC */ u32 scaleY;
    /* 0x500 */ s16 frame;
};

/* Contact sparkles (Ukoback_SpawnContactSparkle, UkobackSparkle_Update):
 * 16.16 offset from the player.
 */
struct UkobackSparkleWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 offsetX;
    /* 0x4FC */ u32 offsetY;
};

/* Death pieces (Ukoback_SpawnDeathPiece, UkobackDeathPiece_Update): the
 * Ukoback's entity index (its index halfword copied).
 */
struct UkobackPieceWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ s16 ownerIndex;
};

/* Work slots of Alastor (same layout as Nemesis, see nemesis_entity.h): -
 * angle: heading / affine rotation, turn: rotation added per frame; - flags:
 * state bits shared by the attack routines (2, 0x20, ...); - part index:
 * entity index of its linked part (Alastor_CreateLinkedPart); - hits before
 * react: hits it absorbs before the hit reaction (reset to 3); - wait timer:
 * random wait while positioning; - attack set / index: attack table
 * selection (Alastor_UpdateAttackSelection).
 * Phase / timer of the current movement (TU aliases PHASE / TIMER): rocking
 * phase while approaching, arc timer, recoil phase.
 */
struct AlastorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 waitTimer;
    /* 0x4FA */ u8 attackSet;
    /* 0x4FB */ u8 attackIndex;
    /* 0x4FC */ s16 flags;
    /* 0x4FE */ s16 phase;
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 turn;
    /* 0x504 */ u8 pad_504[0x508 - 0x504];
    /* 0x508 */ u8 hitsBeforeReact;
};

/* Work slots of Alastor (same layout as Nemesis, see nemesis_entity.h): -
 * angle: heading / affine rotation, turn: rotation added per frame; - flags:
 * state bits shared by the attack routines (2, 0x20, ...); - part index:
 * entity index of its linked part (Alastor_CreateLinkedPart); - hits before
 * react: hits it absorbs before the hit reaction (reset to 3); - wait timer:
 * random wait while positioning; - attack set / index: attack table
 * selection (Alastor_UpdateAttackSelection).
 */
struct AlastorPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 index;
};

/* Pattern attack (Alastor_UpdatePatternAttack, TU alias INDEX): segment of
 * the movement pattern; reset by Alastor_UpdateSpinningLeap.
 */
struct AlastorPatternWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 step;
};

/* Grab attack slots of the Succubus (Succubus_UpdateAttack): - lunging: set
 * while it dashes at the player; contact then grabs instead of dealing
 * contact damage (Succubus_DamagePlayerOnContact); - grabbed / holding: set
 * by that contact; while holding there is no contact damage check; - escape
 * presses: buttons pressed by the player while held (frees at 30); - drain
 * timer / drain count: frames until the next drain hit and the number of
 * hits dealt (the hold ends after 5).
 * Walk (Succubus_UpdateWalk; same template as Lilith): provoked (set when
 * hit, makes it attack at once), seen (the target came near, sound 0x1C7
 * played once) and dodge checked (the dodge chance for the current player
 * attack was already rolled).
 * Intro: set when it starts in its intro pose (spawn parameter 0, cleared
 * when the intro animation ends); a hit then only sets holding and cannot
 * bring its HP below 1 (Succubus_OnHit).
 */
struct SuccubusWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 provoked;
    /* 0x4FD */ u8 dodgeChecked;
    /* 0x4FE */ u8 intro;
    /* 0x4FF */ u8 holding;
    /* 0x500 */ u8 lunging;
    /* 0x501 */ s8 grabbed;
    /* 0x502 */ u8 escapePresses;
    /* 0x503 */ u8 drainCount;
    /* 0x504 */ u8 drainTimer;
    /* 0x505 */ u8 seen;
};

/* Werejaguar work slots: - manual anim: nonzero = the attack code steps the
 * animation itself (EnemyWerejaguarUpdate skips Anim_Update); - contact
 * kind: which attack the contact damage belongs to (0 = body, 1 = claw 3/4
 * power, 2 = knock back, 3 = strong knock back); - contact hit: set by
 * Werejaguar_DamagePlayerOnContact after a hit.
 * Cleared when the attack starts; never read.
 */
struct WerejaguarWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 manualAnim;
    /* 0x4F9 */ u8 contactKind;
    /* 0x4FA */ u8 unusedFlag;
    /* 0x4FB */ u8 contactHit;
};

/* Death effect (WerejaguarDeathEffect_Update): sway angle, screen x / y and
 * the camera x / y at spawn (keeps it fixed in the room), base sprite layer
 * (drawn one layer in front of / behind it).
 */
struct WerejaguarEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ u32 layer;
    /* 0x500 */ s16 x;
    /* 0x502 */ s16 cameraX;
    /* 0x504 */ s16 y;
    /* 0x506 */ s16 cameraY;
};

/* Big Golem parts (BigGolemPart_Update; joint slots in joint_entity.h): the
 * owner (whose unk_500.bigGolemParts holds the joint array) and, once the
 * golem is dead, the falling piece's spin, affine angle and floor y.
 */
struct BigGolemPartWork {
    /* 0x4F4 */ s32 spin;
    /* 0x4F8 */ u8 pad_4F8[0x4FC - 0x4F8];
    /* 0x4FC */ u32 angle;
    /* 0x500 */ struct EwramData_EntityData *owner;
    /* 0x504 */ u8 pad_504[0x508 - 0x504];
    /* 0x508 */ s32 floorY;
};

/* Body: hit / hit count (set and incremented, up to 10, by
 * BigGolem_OnDamaged) and its two palette slots.
 * Walk (GreatArmor-like TU macros WALL_CHECK / RANDOM_TURN of
 * BigGolem_UpdateWalk): heading the golem walks in (compared with flipX; set
 * to flipX ^ 1 to turn around at walls, at random near the target or after
 * more than 3 hits).
 * Parts: heap block of its 14 part records (struct BigGolemPart).
 */
struct BigGolemWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 hitCount;
    /* 0x4FE */ u8 hit;
    /* 0x4FF */ u8 pad_4FF[0x500 - 0x4FF];
    /* 0x500 */ struct BigGolemPart *parts;
    /* 0x504 */ u8 pad_504[0x505 - 0x504];
    /* 0x505 */ u8 heading;
    /* 0x506 */ u8 pad_506[0x50A - 0x506];
    /* 0x50A */ u8 palette0;
    /* 0x50B */ u8 palette1;
};

/* Attack (BigGolem_UpdateAttack - TU alias VARIANT): punch variant 0 / 1. */
struct BigGolemAttackWork {
    /* 0x4F4 */ u8 variant;
};

/* Creaking Skull (EnemyCreakingSkull*, CreakingSkull_*; joint slots in
 * joint_entity.h): - variant: index into the per-variant timing tables (0 =
 * enemy 0x21, 1 = enemy 0x5C); - pose delay: frames until the mid-step pose
 * switch (half the step timer).
 * Unused flag: cleared at the start of every update, never read.
 */
struct CreakingSkullWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 poseDelay;
    /* 0x4FE */ u8 pad_4FE[0x505 - 0x4FE];
    /* 0x505 */ u8 unusedFlag;
    /* 0x506 */ u8 pad_506[0x508 - 0x506];
    /* 0x508 */ s32 variant;
};

/* Bone part after the skull dies (CreakingSkullPart_Update): spin speed,
 * floor y it bounces on, and its affine angle.
 */
struct CreakingSkullPartWork {
    /* 0x4F4 */ s32 spin;
    /* 0x4F8 */ s32 floorY;
    /* 0x4FC */ u32 angle;
};

/* Guided effect (CreakingSkullGuidedEffect_Update): room positions it heads
 * for, first the early target, in its last 7 frames the late target.
 */
struct CreakingSkullGuideWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 lateX;
    /* 0x4FC */ u32 lateY;
    /* 0x500 */ s32 earlyX;
    /* 0x504 */ s32 earlyY;
};

/* Bone projectile (CreakingSkullProjectile_Update): angle of its orbiting
 * particles.
 */
struct CreakingSkullProjectileWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 orbit;
};

/* Cockatrice / Basilisk (CockatriceFamily*): - body: palette slot shared
 * with its part and attack effect; death flags (bit 0 set when the death
 * sequence starts, ends the part).
 */
struct CockatriceWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 paletteSlot;
    /* 0x4F9 */ u8 pad_4F9[0x4FC - 0x4F9];
    /* 0x4FC */ u32 deathFlags;
};

/* Swinging affine part (CockatriceFamilyPart_Update): affine angle, swing
 * phase, x scale and swing amplitude (8, 16 while the body attacks).
 */
struct CockatricePartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 phase;
    /* 0x500 */ s32 scaleX;
    /* 0x504 */ s16 amplitude;
};

/* Petrifying beam (CockatriceFamilyAttack_*): affine angle, frame counter
 * (ends after 32 frames), y scale, length (scale of the step) and the step
 * between the 16 copies drawn by CockatriceFamilyAttack_Draw.
 */
struct CockatriceBeamWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 frame;
    /* 0x500 */ s32 scaleY;
    /* 0x504 */ u8 pad_504[0x508 - 0x504];
    /* 0x508 */ s32 length;
    /* 0x50C */ s32 stepX;
};

/* Event scene 3 controller (sEventSceneCreateFuncs[3], EventScene03_*): -
 * actor 0 / 1 / 8: the helper entities created from the object table
 * (Entity_CreateFromObjectTable kinds 0, 1 and 8); actor 0 is placed next to
 * the player, actor 1 walks in and plays the attack animation, actor 8
 * becomes EventScene03MainPart_Update and finally turns into soul orbs and
 * debris; - progress: sequence bits read by the parts through their parent
 * index: 8 = parts start moving, 0x10 = actor 1 attack started, 2 = its
 * animation finished, 4 = scene over (parts end).
 */
struct Event03Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *actor0;
    /* 0x4FC */ struct EwramData_EntityData *actor1;
    /* 0x500 */ struct EwramData_EntityData *actor8;
    /* 0x504 */ s16 progress;
};

/* Homing orbs (EventScene03_SpawnSoulOrb / _SpawnTargetOrb and their
 * updates): heading angle, turn rate (grows by 42 up to 0x4000; the soul orb
 * starts with its mode), 16.16 speed (grows to 0x80000) and the target
 * entity of the target orb.
 */
struct Event03OrbWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 heading;
    /* 0x500 */ s32 turnRate;
    /* 0x504 */ s32 speed;
    /* 0x508 */ struct EwramData_EntityData *target;
};

/* Parts (EventScene03_Spawn*Part, *Part_Update): spawn flags (bit 1 hidden,
 * 4 / 8 / 0x10 variant choices), entity index of the linked part and the
 * wave phase of the floating motion.
 */
struct Event03PartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 linkIndex;
    /* 0x4F7 */ u8 flags;
    /* 0x4F8 */ u8 pad_4F8[0x50A - 0x4F8];
    /* 0x50A */ s16 wave;
};

/* Work slots of the Slime enemy (crawls along floors, walls and ceilings): -
 * speed: signed crawl speed along the surface (the sign is the direction); -
 * angle: surface angle (affine rotation), turned towards
 * sSlimeSurfaceTurnTargets[turn surface] by the turn rate per frame; - turn
 * cooldown: frames after a turn before the next surface check; - flags: 1 =
 * was hit (start the hurt animation), 2 = hurt animation playing, 4 =
 * detached and falling; - scale x / y: affine scale (squash while crawling
 * and dying); - front / back support: room coordinates of the two support
 * probes (Slime_UpdateSurfaceSupport).
 */
struct SlimeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F5 - 0x4F4];
    /* 0x4F5 */ u8 flags;
    /* 0x4F6 */ u8 turnCooldown;
    /* 0x4F7 */ u8 turnSurface;
    /* 0x4F8 */ s32 speed;
    /* 0x4FC */ u32 scaleX;
    /* 0x500 */ s32 scaleY;
    /* 0x504 */ s16 angle;
    /* 0x506 */ s16 turnRate;
    /* 0x508 */ s16 frontSupportX;
    /* 0x50A */ s16 frontSupportY;
    /* 0x50C */ s16 backSupportX;
    /* 0x50E */ s16 backSupportY;
};

/* Work slots of Nemesis (and of its partner entity, which runs the same
 * code): - angle: heading / affine rotation, turn: rotation added per frame;
 * - flags: state bits shared by the attack routines (2, 0x20, ...); -
 * partner index: entity index of the partner (Nemesis_CreatePartner); -
 * afterimage 0..2: entity slots of its three afterimages; - hits before
 * react: hits it absorbs before the hit reaction (reset to 3); - wait timer:
 * random wait while positioning; - attack set / index: attack table
 * selection (Nemesis_UpdateAttackSelection).
 * Phase / timer of the current movement (TU aliases PHASE / TIMER): rocking
 * phase while approaching, arc timer, recoil phase.
 */
struct NemesisWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 partnerIndex;
    /* 0x4F7 */ u8 pad_4F7[0x4F8 - 0x4F7];
    /* 0x4F8 */ u16 waitTimer;
    /* 0x4FA */ u8 attackSet;
    /* 0x4FB */ u8 attackIndex;
    /* 0x4FC */ s16 flags;
    /* 0x4FE */ s16 phase;
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 turn;
    /* 0x504 */ u8 pad_504[0x505 - 0x504];
    /* 0x505 */ u8 afterimage0;
    /* 0x506 */ s8 afterimage1;
    /* 0x507 */ u8 afterimage2;
    /* 0x508 */ u8 hitsBeforeReact;
};

/* Graham family state flags: set on the main entity and copied every frame
 * to the body parts through their owner pointers (2 = first form hit
 * reaction, 4 / 8 = transition steps, 0x10 / 0x20 = attack phases read by
 * the parts, 0x40, 0x80 = scene objects created). Body overlay: its two side
 * part entities (Graham_CreateSidePart 0 / 1). Side part: which side it is
 * on (toggled during the attack), the step in its wait table and a flash
 * countdown.
 */
struct GrahamWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FA - 0x4F4];
    /* 0x4FA */ s16 flags;
    /* 0x4FC */ u8 pad_4FC[0x50C - 0x4FC];
    /* 0x50C */ s32 sidePart0;
};

/* Graham family state flags: set on the main entity and copied every frame
 * to the body parts through their owner pointers (2 = first form hit
 * reaction, 4 / 8 = transition steps, 0x10 / 0x20 = attack phases read by
 * the parts, 0x40, 0x80 = scene objects created). Body overlay: its two side
 * part entities (Graham_CreateSidePart 0 / 1). Side part: which side it is
 * on (toggled during the attack), the step in its wait table and a flash
 * countdown.
 */
struct GrahamPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 step;
    /* 0x4FE */ s16 side;
    /* 0x500 */ s16 flash;
};

/* Scene links of the root Graham entity: entity index of the scene object
 * (object 0x20, Graham_CreateSceneObject), of the temporary intro helper and
 * of the projectile tracked in the defeat scene. Graham's form entities keep
 * the entity index of their linked child (combat body, attack effect,
 * transition part) in child index.
 */
struct GrahamSceneWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 childIndex;
    /* 0x4F7 */ u8 helperIndex;
    /* 0x4F8 */ s8 objectIndex;
    /* 0x4F9 */ u8 pad_4F9[0x4FE - 0x4F9];
    /* 0x4FE */ s16 defeatProjectileIndex;
};

/* First form attacks (GrahamFirstForm_UpdateAttacks): sub-state a volley
 * resumes after turning to face the player, and which side the double volley
 * starts with.
 */
struct GrahamTurnWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ u8 resume;
    /* 0x501 */ u8 pad_501[0x507 - 0x501];
    /* 0x507 */ u8 volleySide;
};

/* Scene (Graham_SpawnSceneProjectile): emitter counter, cycles through the
 * five spawn points.
 */
struct GrahamEmitterWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 counter;
};

/* Gladiator (EnemyGladiator*, Gladiator_*): flinch flag (set when damaged,
 * stops the box animation) and palette slot.
 * Second palette variant (variant 1 of sGladiatorPalette, the base palette).
 */
struct GladiatorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 flinch;
    /* 0x4F9 */ u8 pad_4F9[0x504 - 0x4F9];
    /* 0x504 */ u8 palette;
    /* 0x505 */ u8 palette1;
};

/* Spinning weapon parts (GladiatorSpinner_Update, created in
 * EnemyGladiatorCreate): part index, pixel offset from the owner, affine
 * angle and spin speed (accelerates up to 0x800).
 */
struct GladiatorSpinnerWork {
    /* 0x4F4 */ s16 offsetX;
    /* 0x4F6 */ s16 offsetY;
    /* 0x4F8 */ u8 pad_4F8[0x4FC - 0x4F8];
    /* 0x4FC */ s8 index;
    /* 0x4FD */ u8 pad_4FD[0x500 - 0x4FD];
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 spin;
};

/* Body and arm parts (GladiatorBody_Update, GladiatorArm_Update): pixel
 * offset from the owner, vertical bob of the owner's animation step, and the
 * arm's random idle timer. Shot (GladiatorShot_Update): heading, turned
 * towards straight down by 0x100 per frame.
 */
struct GladiatorPartWork {
    /* 0x4F4 */ s16 offsetX;
    /* 0x4F6 */ s16 offsetY;
    /* 0x4F8 */ u8 pad_4F8[0x4FE - 0x4F8];
    /* 0x4FE */ s16 bobY;
};

/* Body and arm parts (GladiatorBody_Update, GladiatorArm_Update): pixel
 * offset from the owner, vertical bob of the owner's animation step, and the
 * arm's random idle timer. Shot (GladiatorShot_Update): heading, turned
 * towards straight down by 0x100 per frame.
 */
struct GladiatorArmWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s16 timer;
};

/* Body and arm parts (GladiatorBody_Update, GladiatorArm_Update): pixel
 * offset from the owner, vertical bob of the owner's animation step, and the
 * arm's random idle timer. Shot (GladiatorShot_Update): heading, turned
 * towards straight down by 0x100 per frame.
 * Unused dir: facing of the arm at the shot, never read.
 */
struct GladiatorShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 unusedDir;
    /* 0x4FE */ u8 pad_4FE[0x502 - 0x4FE];
    /* 0x502 */ s16 angle;
};

/* Altair work slots: - carried: the enemy it carries
 * (Altair_CreateCarriedEnemy); carrying: 1 while it still holds it; drop
 * ready: set when it decides to drop it near the target; - flight phase:
 * 0..3 step of Altair_UpdateFlight (3 = dying); - spawner: the spawner
 * entity that created it; on the spawner, spawn count is the number of live
 * Altairs (decremented when one is deleted).
 */
struct AltairWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *carried;
    /* 0x4FC */ s8 flightPhase;
    /* 0x4FD */ u8 pad_4FD[0x500 - 0x4FD];
    /* 0x500 */ u8 carrying;
    /* 0x501 */ s8 dropReady;
    /* 0x502 */ s16 spawnCount;
    /* 0x504 */ struct EwramData_EntityData *spawner;
};

/* Drop counter: Altair_CreateCarriedEnemy counts each carried enemy on the
 * Altair spawner (drop count) and marks the enemy (drop counted, with the
 * spawner's entity index); the enemy decrements the count when it dies.
 */
struct AltairDropWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ s16 spawnerIndex;
    /* 0x4F8 */ u8 pad_4F8[0x4FD - 0x4F8];
    /* 0x4FD */ u8 count;
};

/* Feathers (AltairFeather_Update): the Altair that shed it, sway speed
 * (random 0x40..0xBF) and the sway phase it advances (ends after half a
 * turn).
 * Feather: random spin and a speed set when it starts to fall; never read.
 */
struct AltairFeatherWork {
    /* 0x4F4 */ struct EwramData_EntityData *owner;
    /* 0x4F8 */ u16 swaySpeed;
    /* 0x4FA */ s16 swayPhase;
    /* 0x4FC */ u8 pad_4FC[0x508 - 0x4FC];
    /* 0x508 */ s16 unusedSpin;
    /* 0x50A */ s16 unusedSpeed;
};

/* Devil main entity: - target x / y: room position it flies to (beside the
 * target, random height), speed: its approach speed (eased up to 0x20000); -
 * timer: frame counter of the current approach / attack step; - intangible:
 * nonzero = no contact damage check (parts also skip their hit checks unless
 * it is 1); - parts hidden: nonzero = the body parts are not drawn.
 * Parts: heap block of its 8 part records (struct DevilPart).
 */
struct DevilWork {
    /* 0x4F4 */ s32 targetX;
    /* 0x4F8 */ s32 targetY;
    /* 0x4FC */ u32 speed;
    /* 0x500 */ struct DevilPart *parts;
    /* 0x504 */ u8 pad_504[0x508 - 0x504];
    /* 0x508 */ s16 timer;
    /* 0x50A */ u8 intangible;
};

/* Devil main entity: - target x / y: room position it flies to (beside the
 * target, random height), speed: its approach speed (eased up to 0x20000); -
 * timer: frame counter of the current approach / attack step; - intangible:
 * nonzero = no contact damage check (parts also skip their hit checks unless
 * it is 1); - parts hidden: nonzero = the body parts are not drawn.
 */
struct DevilPartsWork {
    /* 0x4F4 */ u8 pad_4F4[0x50B - 0x4F4];
    /* 0x50B */ u8 hidden;
};

/* Parts (DevilPart_Update): affine angle (from the joint direction). */
struct DevilPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 angle;
};

/* Projectile (DevilProjectile_Spawn / _Update): owner, 16.16 offset from the
 * owner it is held at, and the frame (timer value) it is released on.
 */
struct DevilProjectileWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 offsetX;
    /* 0x4FC */ u32 offsetY;
    /* 0x500 */ struct EwramData_EntityData *owner;
    /* 0x504 */ u8 release;
};

/* Charge effect (DevilChargeEffect_Spawn / _Update): 16.16 offset from the
 * Devil.
 * Charge effect (DevilChargeEffect_Spawn / _Update): the Devil it follows.
 */
struct DevilChargeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 offsetX;
    /* 0x4FC */ u32 offsetY;
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Work slots of the Skeleton enemy (EnemySkeletonCreate /
 * EnemySkeletonUpdate): - home x / patrol left / patrol right: room x of the
 * spawn point and the patrol bounds (home +- 0x20); - affine / spin angle:
 * its affine slot and the rotation angle advanced every frame; - flags: part
 * and state bits (1 = linked part created, 4 = attached part created, 2 / 8
 * / 0x20 combat variants); - cooldown: frames before the next attack check;
 * - walk speed: 16.16 walking speed (damped every frame); - throw urge:
 * grows every frame in combat; a throw starts when a random value falls
 * below it; throw count: bones thrown in the current attack.
 * EnemySkeletonUpdate stores a random 40..119 here every frame; never read.
 */
struct SkeletonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ s16 unusedDelay;
    /* 0x4F8 */ u16 patrolRight;
    /* 0x4FA */ s16 patrolLeft;
    /* 0x4FC */ s8 throwCount;
    /* 0x4FD */ u8 throwUrge;
    /* 0x4FE */ u8 flags;
    /* 0x4FF */ u8 pad_4FF[0x500 - 0x4FF];
    /* 0x500 */ s16 cooldown;
    /* 0x502 */ s16 spinAngle;
    /* 0x504 */ s16 affine;
    /* 0x506 */ s16 homeX;
    /* 0x508 */ s32 walkSpeed;
};

/* Death pieces (Skeleton_SpawnDeathPieces, SkeletonDeathPiece_UpdateUnused):
 * the dead skeleton counts its live pieces in death pieces (the throw count
 * slot); a piece keeps the skeleton's entity index and its piece index (row
 * of sSkeletonDeathPieceOffsets, stored in the parent-index slot).
 */
struct SkeletonDeathWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 pieces;
};

/* Death pieces (Skeleton_SpawnDeathPieces, SkeletonDeathPiece_UpdateUnused):
 * the dead skeleton counts its live pieces in death pieces (the throw count
 * slot); a piece keeps the skeleton's entity index and its piece index (row
 * of sSkeletonDeathPieceOffsets, stored in the parent-index slot).
 */
struct SkeletonPieceWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F5 - 0x4F4];
    /* 0x4F5 */ u8 index;
    /* 0x4F6 */ u8 pad_4F6[0x4F8 - 0x4F6];
    /* 0x4F8 */ s8 ownerIndex;
};

/* Thrown bone (SkeletonProjectile_Update): kind (1 = the variant thrown with
 * flag 2: frame 0x1B, sound 0x89).
 */
struct SkeletonBoneWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 kind;
};

/* Witch (Witch_*): flags (bit 0 = invulnerable while charging, bit 1 =
 * casting, bit 2 = wait set up; Witch_UpdateCast / _UpdateCharge write the
 * byte through a cast pointer) and the 16.16 point it flies to.
 */
struct WitchWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 flags;
    /* 0x4F9 */ u8 pad_4F9[0x500 - 0x4F9];
    /* 0x500 */ s32 moveX;
    /* 0x504 */ s32 moveY;
};

/* Spell (WitchSpell_*): affine scale, angle and spin, homing delay, the room
 * point it flies to and the entity index of the casting witch.
 */
struct WitchSpellWork {
    /* 0x4F4 */ s32 scale;
    /* 0x4F8 */ u16 angle;
    /* 0x4FA */ s16 spin;
    /* 0x4FC */ s16 delay;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s32 targetX;
    /* 0x504 */ s32 targetY;
    /* 0x508 */ u8 pad_508[0x50C - 0x508];
    /* 0x50C */ u8 ownerIndex;
};

/* Orbit particle (WitchOrbitParticle_Update): radius, angle and spin. */
struct WitchOrbitWork {
    /* 0x4F4 */ s32 radius;
    /* 0x4F8 */ u16 angle;
    /* 0x4FA */ s16 spin;
};

/* Ground projectile (WitchGroundProjectile_Update): walk speed. */
struct WitchGroundWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 speed;
};

/* Cast (Witch_UpdateCast - TU alias TIMER): frames of the current cast step. */
struct WitchCastWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 timer;
};

/* Disc Armor (EnemyDiscArmorUpdate, DiscArmor_UpdateThrow): - flags: bit 0 =
 * disc thrown, bit 1 = the disc is back (set by the disc); - hand offset x /
 * y: pixel offset of the throwing hand (-20 / -36 while throwing, eased back
 * to -10 / -16); - hand x / y: 16.16 position of the hand, recomputed every
 * frame; the disc flies around it and the ring aims from it.
 */
struct DiscArmorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 flags;
};

/* Disc Armor (EnemyDiscArmorUpdate, DiscArmor_UpdateThrow): - flags: bit 0 =
 * disc thrown, bit 1 = the disc is back (set by the disc); - hand offset x /
 * y: pixel offset of the throwing hand (-20 / -36 while throwing, eased back
 * to -10 / -16); - hand x / y: 16.16 position of the hand, recomputed every
 * frame; the disc flies around it and the ring aims from it.
 */
struct DiscArmorHandWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 x;
    /* 0x500 */ s32 y;
    /* 0x504 */ s16 offsetX;
    /* 0x506 */ s16 offsetY;
};

/* Disc (DiscArmorDisc_Update): owner and ring entity, affine angle, orbit
 * angle around the hand, its turn step, orbit radius, a per-state counter
 * (spiral phase while flying out, rolling speed on the floor, bounce speed
 * when dropped), and the x / y movement of the last frame (the y one is kept
 * in the pointer-typed unk_514 field).
 */
struct DiscArmorDiscWork {
    /* 0x4F4 */ struct EwramData_EntityData *owner;
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ struct EwramData_EntityData *ring;
    /* 0x500 */ s32 orbitAngle;
    /* 0x504 */ s32 turn;
    /* 0x508 */ s32 radius;
    /* 0x50C */ s32 phase;
};

/* Ring (DiscArmorRing_Update / _Aim / _Draw): angle from the hand to the
 * disc, their distance (sets the ring scale) and the step between the three
 * drawn copies.
 */
struct DiscArmorRingWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ u32 stepX;
    /* 0x500 */ s32 stepY;
    /* 0x504 */ s32 length;
};

/* Sparks (SpawnSpark in code_080B1678.c, DiscArmorSpark_Update): animation. */
struct DiscArmorSparkWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 anim;
};

/* Man Eater heads (created by EnemyManEaterUpdate, ManEaterHead_Update): -
 * owner index: entity index of the Man Eater body; - orbit slot: which
 * sManEaterHeadOrbitPoints entry the head uses (also the index of a neck
 * link); - flags: 1 = lunging, 2 = may lunge again, 4 = neck links created;
 * - angle / home angle: current angle of the head (kept within +-0x2000 of
 * the home angle); base angle: angle offset added to the body rotation; -
 * lunge speed: 16.16 extension speed (grows to 0x60000). Neck links keep the
 * head index (owner index), the previous link index, the base entity index,
 * their slot and their angle.
 */
struct ManEaterWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F5 - 0x4F4];
    /* 0x4F5 */ u8 headFlags;
    /* 0x4F6 */ u8 pad_4F6[0x4F8 - 0x4F6];
    /* 0x4F8 */ s8 ownerIndex;
    /* 0x4F9 */ u8 prevIndex;
    /* 0x4FA */ u8 baseIndex;
    /* 0x4FB */ u8 orbitSlot;
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 homeAngle;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ s32 lungeSpeed;
    /* 0x508 */ s16 baseAngle;
};

/* Body (EnemyManEaterCreate / EnemyManEaterUpdate): number of heads still
 * alive (3 at first; dies at 0), its affine angle and the sway phase it is
 * computed from.
 */
struct ManEaterBodyWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ u8 headCount;
    /* 0x501 */ u8 pad_501[0x502 - 0x501];
    /* 0x502 */ s16 swayPhase;
};

/* Neck links (ManEaterSegment_Update; head / orbit slot in OWNER_INDEX /
 * ORBIT_SLOT, the swing angle in ext.manEater.angle): swing speed, state saved
 * while the head lunges, and the flag of the tip link (slot 5) that drops
 * projectiles once the head is below 200 HP.
 * Neck links: set by ManEaterSegment_Update at creation, never read for
 * links.
 */
struct ManEaterLinkWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FE - 0x4F4];
    /* 0x4FE */ s16 swingSpeed;
    /* 0x500 */ u8 savedState;
    /* 0x501 */ s8 drops;
    /* 0x502 */ s16 unusedPhase;
    /* 0x504 */ s32 unusedSpeed;
};

/* Pickups (Pickup_Update): bob frame (the low byte drives the sine bob),
 * lifetime (300 frames, then it disappears) and the delay before the player
 * can collect it.
 */
struct PickupWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ union { s32 frame; s8 low; } bob;
    /* 0x4FC */ s16 lifetime;
    /* 0x4FE */ s16 delay;
};

/* Alura Une (AluraUne_*): awake flag (set the first time the player comes
 * near or it is damaged; that first time it attacks at once) and hit flag
 * (set by AluraUne_OnHit, ends the idle wait; cleared by the attacks).
 */
struct AluraUneWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ u8 awake;
    /* 0x505 */ u8 hit;
};

/* Alura Une thorns (AluraUne_SpawnThorn, AluraUneThorn_Update): frames they
 * stay out.
 */
struct AluraUneThornWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s16 time;
};

/* Alura Une root wave (AluraUneRootWave_Update): jumps done (turns after the
 * third, deleted after six).
 */
struct AluraUneRootWaveWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 step;
};

/* Afterimages (Afterimage_Create, Afterimage_Update): owner, heap buffer of
 * the owner's recorded positions (24 entries; as a pointer in the update),
 * start delay, and flags (4 / 0x10 / 0x80 drawing options, 0x20 = end when
 * the owner's hp runs out).
 */
struct AfterimageWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ union { s32 value; struct EwramData_EntityData *ptr; } history;
    /* 0x4FC */ struct EwramData_EntityData *owner;
    /* 0x500 */ u8 pad_500[0x502 - 0x500];
    /* 0x502 */ s16 delay;
    /* 0x504 */ s32 flags;
};

/* Une spawner (the room entity, EnemyUneCreate): number of live plants,
 * limited to its spawnParam0.
 */
struct UneSpawnerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 count;
};

/* Une plants: the spawner they belong to, original = 1 for the first plant,
 * spread timer = frames until the next plant grows.
 */
struct UneWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *spawner;
    /* 0x500 */ s32 original;
    /* 0x504 */ s32 spreadTimer;
};

/* Winged Skeleton spawner (EnemyWingedSkeletonUpdate): number of live
 * skeletons it created.
 */
struct WingedSkeletonSpawnerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 childCount;
};

/* Winged Skeleton: the spawner (its count is decremented when the skeleton
 * is deleted); blocked = the tile ahead in the dive direction is solid, the
 * skeleton then turns instead of tracking the target
 * (WingedSkeleton_UpdateFly).
 */
struct WingedSkeletonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *spawner;
    /* 0x500 */ u8 blocked;
};

/* Zombie (EnemyZombie*, Zombie_*): timer = frames it walks before sinking
 * back (random 600..1111), then the sinking frame counter; debris / sprite
 * spawned = the falling debris and the temporary sprite of the current
 * animation step were spawned; spawner = the zombie spawner it belongs to.
 */
struct ZombieWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 debrisSpawned;
    /* 0x4F9 */ u8 spriteSpawned;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ struct EwramData_EntityData *spawner;
    /* 0x500 */ s32 timer;
};

/* Zombie spawner: number of live zombies. */
struct ZombieSpawnerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 childCount;
};

/* Body pieces of Zombie / Zombie Officer / Zombie Soldier (*_SpawnPiece,
 * *Piece_Update): floor y where the falling piece stops.
 */
struct ZombiePieceWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 floorY;
};

/* Zombie temporary sprite (Zombie_SpawnTemporarySprite): lifetime in frames. */
struct ZombieSpriteWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 lifetime;
};

/* Triton (Triton_UpdateMove / _UpdateAttack - TU aliases NEXT_STATE /
 * SPEAR_FLAG): its spear, the sub-state the move code continues with, and
 * the thrust flag - set on the Triton to raise the spear, set on the spear
 * (through this view) by TritonSpear_Update when the thrust is done (each
 * side clears the other).
 */
struct TritonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 thrust;
    /* 0x4FD */ u8 nextState;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ struct EwramData_EntityData *spear;
};

/* Triton spear (Triton_CreateSpear, TritonSpear_Update - TU alias
 * PART_ANGLE): the Triton holding it and its affine angle.
 */
struct TritonSpearWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *owner;
    /* 0x504 */ s32 angle;
};

/* Triton homing shot: the Triton that fired it. */
struct TritonShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Triton wave particles (TU aliases WAVE_ANGLE / WAVE_AGE): sway angle and
 * age.
 */
struct TritonWaveWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ u8 pad_4FC[0x500 - 0x4FC];
    /* 0x500 */ s32 age;
};

/* Merman (EnemyMerman*, Merman_*): leap / land splashed = the water-exit and
 * water-entry splash effects of the current jump were spawned; swim mode: 0
 * = wander, 1 = face the target; walk count = walk turns on land (back into
 * the water after 8); home x / y = room pixel position of the spawn point
 * (the water line); spawner = the merman spawner it belongs to; unused mode
 * = spawn mode stored by EnemyMermanCreate (1 when it starts walking), never
 * read.
 */
struct MermanWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 leapSplashed;
    /* 0x4F9 */ u8 swimMode;
    /* 0x4FA */ u8 landSplashed;
    /* 0x4FB */ u8 walkCount;
    /* 0x4FC */ struct EwramData_EntityData *spawner;
    /* 0x500 */ s16 homeX;
    /* 0x502 */ s16 homeY;
    /* 0x504 */ u8 unusedMode;
};

/* Merman spawner (spawnParam1 != 0): number of live children. */
struct MermanSpawnerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 childCount;
};

/* Gorgon: flags (bit 0 = show the forced frame instead of the animation,
 * cleared every update), walk gait and frame clock (Gorgon_UpdateWalk TU
 * aliases GAIT / CLOCK; cleared after the breath attack), the forced sprite
 * frame, hit (set by Gorgon_OnHit, cleared by the update), and the breath
 * attack: it sweeps its angle by 0x400 per frame and reverses the direction
 * every 7 steps. The walk code also reads the four bytes at 0x4FC as one
 * word and 0x4FE..0x4FF as one halfword (GORGON_WALK_WORD /
 * GORGON_FRAME_CLOCK in gorgon_entity.h).
 */
struct GorgonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 flags;
    /* 0x4FD */ u8 gait;
    /* 0x4FE */ u8 forcedFrame;
    /* 0x4FF */ u8 clock;
    /* 0x500 */ u8 hit;
    /* 0x501 */ u8 pad_501[0x504 - 0x501];
    /* 0x504 */ u8 breathDir;
    /* 0x505 */ u8 breathStep;
    /* 0x506 */ s16 breathAngle;
};

/* Great Armor (EnemyGreatArmor*, GreatArmor_*; joint slots in
 * joint_entity.h): variant = index into the per-variant timing tables (0..2
 * by enemy id; the TU macros VARIANT read it as s8); hit / hit count: set
 * and incremented by GreatArmor_OnHit, the flag is cleared by the update
 * (the walk TU reads them as WAS_HIT / HIT_COUNT); attack hit: the TU macro
 * HIT of GreatArmor_UpdateAttack; guard: 1 while the shield arm is raised in
 * the attack states, makes the second hit box harmless (cleared every frame
 * by EnemyGreatArmorUpdate); hop turns: back turns in a hop sequence.
 * Parts: heap block of its 14 part records (struct GreatArmorPart).
 */
struct GreatArmorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 hitCount;
    /* 0x4FE */ u8 hit;
    /* 0x4FF */ u8 hopTurns;
    /* 0x500 */ struct GreatArmorPart *parts;
    /* 0x504 */ u8 pad_504[0x505 - 0x504];
    /* 0x505 */ u8 guard;
    /* 0x506 */ u8 pad_506[0x508 - 0x506];
    /* 0x508 */ u8 variant;
    /* 0x509 */ u8 attackHit;
};

/* Great Armor falling parts after death (GreatArmorPart_Update): spin,
 * affine angle and floor y.
 */
struct GreatArmorPartWork {
    /* 0x4F4 */ s32 spin;
    /* 0x4F8 */ u8 pad_4F8[0x4FC - 0x4F8];
    /* 0x4FC */ u32 angle;
    /* 0x500 */ u8 pad_500[0x508 - 0x500];
    /* 0x508 */ s32 floorY;
};

/* Great Armor slash effect (GreatArmorSlashEffect_*): the armor it follows.
 * Unused kind: 3 stored at the spawn, never read.
 */
struct GreatArmorSlashWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 unusedKind;
    /* 0x4FD */ u8 pad_4FD[0x500 - 0x4FD];
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Manticore: the tail tip entity and the shot attack
 * (Manticore_UpdateAttack): shots fired and shots to fire (1..3). Tail
 * entities keep the Manticore itself in ENTITY_OWNER.
 */
struct ManticoreWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *tailTip;
    /* 0x4FC */ u8 pad_4FC[0x508 - 0x4FC];
    /* 0x508 */ u8 shotCount;
    /* 0x509 */ u8 shotTotal;
};

/* Manticore tail segments (and the tip): anchor part, kind (0..5 segments, 6
 * = tip), updated (set when the segment has been updated this frame; the
 * base segment clears it along the chain and each segment then updates the
 * next one - TU aliases KIND / DONE / NEXT in code_080733D4.c), swing frame
 * counter and step (ManticoreTail_UpdateSwing - TU aliases TIMER / STEP),
 * and the next segment towards the tip or, on the tip, a value used by the
 * swing (initial 0x4000).
 */
struct ManticoreTailWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *anchor;
    /* 0x500 */ u8 kind;
    /* 0x501 */ s8 updated;
    /* 0x502 */ u8 timer;
    /* 0x503 */ u8 step;
    /* 0x504 */ union { s32 value; struct EwramData_EntityData *next; } link;
};

/* Beam Skeleton (same patrol / approach logic as the Skeleton): home x /
 * patrol left / patrol right = spawn x and patrol bounds (+-64); cooldown =
 * frames before the next attack check; walk speed = 16.16 speed (damped
 * every frame); fire urge grows while approaching, firing starts when a
 * random value falls below it; firing: bit 0 = the beam is active; beam
 * level: growing beam scale (0xFFFF = full, the beam then deals damage);
 * death pieces: live death pieces (BeamSkeleton_SpawnDeathPieces); unused
 * delay: EnemyBeamSkeletonUpdate stores a random 40..119 here every frame,
 * never read.
 */
struct BeamSkeletonWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 patrolRight;
    /* 0x4FA */ s16 patrolLeft;
    /* 0x4FC */ s8 deathPieces;
    /* 0x4FD */ u8 fireUrge;
    /* 0x4FE */ u8 firing;
    /* 0x4FF */ u8 pad_4FF[0x500 - 0x4FF];
    /* 0x500 */ s16 cooldown;
    /* 0x502 */ s16 unusedDelay;
    /* 0x504 */ s16 beamLevel;
    /* 0x506 */ s16 homeX;
    /* 0x508 */ s32 walkSpeed;
};

/* Beam Skeleton beam entities: the skeleton's entity index and a frame
 * counter.
 */
struct BeamSkeletonBeamWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 ownerIndex;
    /* 0x4F7 */ u8 pad_4F7[0x4F8 - 0x4F7];
    /* 0x4F8 */ s8 counter;
};

/* Beam Skeleton death pieces (BeamSkeletonDeathPiece_UpdateUnused): the
 * skeleton's entity index and the piece index (TU alias INDEX).
 */
struct BeamSkeletonPieceWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F5 - 0x4F4];
    /* 0x4F5 */ u8 index;
    /* 0x4F6 */ u8 ownerIndex;
};

/* Death (boss): state bits (1 = floating weapon gone, 2 = hit, 4 = weapon
 * attack running, 8 / 0x80 = throw attack steps, 0x10 = wall attack, 0x20 =
 * intro done, 0x40 = weapon damaged), dash step (Death_UpdateDashAttack,
 * 0..5; also the arm pose index of the weapon) and the room pixel point the
 * dash / throw attack flies to.
 * Unused choice: Death_UpdateHover keeps its random attack choice here,
 * never read.
 */
struct DeathWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 flags;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ s8 dashStep;
    /* 0x4FD */ u8 unusedChoice;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s16 targetX;
    /* 0x502 */ s16 targetY;
};

/* Death's helper entities (weapons, blades, pillars, effects) keep the
 * entity index of their owner in this byte instead of the usual
 * ENTITY_PARENT_INDEX byte.
 */
struct DeathHelperWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 ownerIndex;
};

/* Death weapon (DeathWeapon_Update - TU aliases SPIN / SPEED): the secondary
 * (mirror) instance has the mirror flag set and keeps the entity index of
 * the primary weapon; rotation angle and spin speed.
 */
struct DeathWeaponWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F9 - 0x4F4];
    /* 0x4F9 */ u8 primaryIndex;
    /* 0x4FA */ u8 mirror;
    /* 0x4FB */ u8 pad_4FB[0x4FC - 0x4FB];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 spin;
};

/* Death floating weapon: countdown of its current move (same bytes as the
 * mirror flag of deathWeapon).
 */
struct DeathFloatingWeaponWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FA - 0x4F4];
    /* 0x4FA */ s16 timer;
};

/* Death blade (DeathBlade_Update): heading, spin angle and 16.16 speed (to
 * 0x18000).
 */
struct DeathBladeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 heading;
    /* 0x4FE */ s16 spin;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ s32 speed;
};

/* Death pillar (DeathPillar_Update): affine x / y scale. */
struct DeathPillarWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 scaleX;
    /* 0x504 */ s32 scaleY;
};

/* Graham's homing scene projectile (Graham_CreateSceneProjectile,
 * GrahamSceneProjectile_Update): homing flag (steer towards the player), the
 * flight angle, the turn rate per frame (grows to 0x4000) and the 16.16
 * speed (grows to 0x80000); shot pairs (Graham_CreateShotPair): index within
 * the pair (0 = the first).
 */
struct GrahamShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 partIndex;
    /* 0x4F9 */ u8 pad_4F9[0x4FC - 0x4F9];
    /* 0x4FC */ u32 angle;
    /* 0x500 */ s32 turn;
    /* 0x504 */ s32 speed;
    /* 0x508 */ u8 homing;
};

/* Graham spiral projectiles (Graham_SpawnSpiralProjectiles,
 * GrahamSpiralProjectile_Update - TU aliases SCALE / RADIUS / ANGLE): index
 * and count of the ring, sprite rotation and its speed, affine scale, 16.16
 * ring radius (its integer half is read directly,
 * GRAHAM_SPIRAL_RADIUS_PIXELS) and ring angle.
 */
struct GrahamSpiralWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 index;
    /* 0x4F9 */ u8 count;
    /* 0x4FA */ u8 pad_4FA[0x500 - 0x4FA];
    /* 0x500 */ s16 rotation;
    /* 0x502 */ s16 rotationSpeed;
    /* 0x504 */ s32 scale;
    /* 0x508 */ s32 radius;
    /* 0x50C */ s32 angle;
};

/* Graham spread projectiles (Graham_SpawnSpreadProjectiles): index within
 * the volley (0 = the first).
 */
struct GrahamSpreadWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 index;
};

/* White Dragon head (the body segments are drawn from it): history = address
 * of a heap buffer with the recent head angles (segment n uses an older
 * entry, which makes the neck follow the head); anchor x / y = room position
 * of the spawn point (camera position when it was created); offset x / y =
 * head position relative to the anchor; sway = amplitude of the neck wave
 * (eased towards 128 / 256); sway phase = phase of the side-to-side sway;
 * move / shot timer count down to the next move / shot decision while idle
 * (see whiteDragonAim).
 */
struct WhiteDragonWork {
    /* 0x4F4 */ s16 moveTimer;
    /* 0x4F6 */ s16 shotTimer;
    /* 0x4F8 */ u16 offsetX;
    /* 0x4FA */ s16 offsetY;
    /* 0x4FC */ s16 anchorX;
    /* 0x4FE */ s16 anchorY;
    /* 0x500 */ s16 sway;
    /* 0x502 */ s16 swayPhase;
    /* 0x504 */ s32 history;
};

/* White Dragon aim point (same bytes as the move / shot timers): when an
 * attack starts the timers are overwritten with the room pixel point it aims
 * at (target, 20 px up); the attack restarts the timers when it ends.
 */
struct WhiteDragonAimWork {
    /* 0x4F4 */ s16 x;
    /* 0x4F6 */ s16 y;
};

/* White Dragon body segments (EnemyWhiteDragonCreate, WhiteDragonSegment_* -
 * TU aliases SEGMENT): the previous segment towards the head and the segment
 * index (selects the history entry and the distance from the head).
 */
struct WhiteDragonSegmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *prev;
    /* 0x500 */ u8 index;
};

/* White Dragon charge particles (WhiteDragon_SpawnChargeParticle,
 * WhiteDragonChargeParticle_Update): room pixel point they converge on.
 */
struct WhiteDragonChargeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 targetX;
    /* 0x4FA */ s16 targetY;
};

/* Event scene 23 (EventScene23_Init): the actor created from object table
 * kind 1.
 */
struct Event23Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *actor;
};

/* Event scene 24 (EventScene24_Update): the answer of the yes / no question. */
struct Event24Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 answer;
};

/* Event scene 26 (EventScene26_*): the actor created from object table kind
 * 3 (drawn with EventScene26_DrawOffsetCopies) and the text id it shows.
 */
struct Event26Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 textId;
    /* 0x4FC */ struct EwramData_EntityData *actor;
};

/* Event scene 27 (town character, EventScene27_*): text id of the picked
 * conversation and the heal flag (restores HP / MP on the next talk).
 */
struct Event27Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 textId;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ u8 heal;
};

/* Event scene 28 (EventScene28_*): text id of the picked script. */
struct Event28Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 textId;
};

/* Event scene 31 (EventScene31_*): the block holder entity it created,
 * frames shown of the current palette step and the step 0..3
 * (EventScene31_StepPaletteAnim).
 */
struct Event31Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *blockHolder;
    /* 0x4FC */ s16 paletteTick;
    /* 0x4FE */ s16 paletteStep;
};

/* Generic particle effects (Effect_UpdateParticle,
 * Effect_UpdateRandomlyAnimatedParticle, Effect_UpdateTimedParticle,
 * Effect_UpdateParticleWithDamping): flags (bit 0 / bit 1 damp the x / y
 * velocity, bit 2 blink out with Entity_GetVisibilityPattern, bit 3 animate
 * (timed particle) or animate every other update (damped particle)); flip
 * flags (bit 0 / bit 1 randomize flipX / flipY each frame); damping amount
 * of Effect_UpdateParticleWithDamping (the other updates damp by 8).
 */
struct ParticleWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 flags;
    /* 0x4FD */ u8 flipFlags;
    /* 0x4FE */ u8 damping;
};

/* Timed particles drawn by Effect_DrawTimedParticle (flags in ext.particle):
 * lifetime in frames and the mode (0 = rise, 1 = step back through
 * sTimedParticleFrames).
 */
struct ParticleTimedWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 lifetime;
    /* 0x4FE */ u8 stepFrames;
};

/* Double-drawn particles (Effect_UpdateTimedDoubleParticle, save sparkles;
 * drawn by Effect_DrawAtOffsetPair / SaveSparkle_Draw): 16.16 offset of the
 * second copy (random +-16 px).
 */
struct ParticlePairWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 offsetX;
    /* 0x504 */ s32 offsetY;
};

/* Table effects (Effect_CreateFromTable, Effect_UpdateTimedDoubleParticle,
 * Effect_UpdateTableAnimation): index into sUnk_085251A8 (frame, lifetime,
 * animation).
 */
struct ParticleTableWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 index;
};

/* Attached animations (Effect_SpawnAttachedAnimation,
 * Effect_UpdateAttachedAnimation): the entity they follow and the animation
 * index.
 */
struct ParticleAttachWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 anim;
    /* 0x4F9 */ u8 pad_4F9[0x4FC - 0x4F9];
    /* 0x4FC */ struct EwramData_EntityData *parent;
};

/* Player effect sets (Player_SpawnEffectSet, PlayerEffectSetPart_Update):
 * animation index of the part.
 */
struct PlayerEffectSetWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 anim;
};

/* Player surface effects (Player_SpawnSurfaceEffect,
 * Effect_UpdatePlayerSurfaceEffect): animation index from
 * sPlayerSurfaceEffects.
 */
struct SurfaceEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 anim;
};

/* Hit effects (HitEffect_Spawn, HitEffect_SpawnAtContact, the hit spark of
 * ResolveDamage): effect type (0 plain hit, 1..6 attribute index); also
 * stored in subState, which HitEffect_Update uses.
 */
struct HitEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 type;
};

/* Player flash (Effect_CreatePlayerFlash, Effect_DrawPlayerFlash): its row
 * of sPlayerFlashInfo, the kind (table index) and the duration in frames
 * (info byte 1; the game is paused while it grows).
 */
struct PlayerFlashWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 info;
    /* 0x4FC */ s8 kind;
    /* 0x4FD */ u8 duration;
};

/* Fragment sets (Entity_SpawnFragmentSet, FragmentSet_Update): the owner
 * counts its live fragments.
 */
struct FragmentSetWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 count;
};

/* Soul acquisition (SoulAcquisition_CreateEffect / _DrawAndWait): frames it
 * holds the game paused (0x40).
 */
struct SoulAcquireWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FD - 0x4F4];
    /* 0x4FD */ u8 duration;
};

/* Legion main entity (EnemyLegionCreate / EnemyLegionUpdate): data = address
 * of its heap block (struct LegionState: room position, rotation, bob phase,
 * shell states; read as a pointer when freed), freed when it dies; circle
 * dir: 1 / 2 = which way it circles the room edges (chosen towards the
 * target), retarget timer = frames until the direction is chosen again;
 * flags: 1 = no contact damage, 2 = bob up and down, 4 = rotate the affine
 * sprite, 8 / 0x10 / 0x20 = shell part ring 1..3 spawned; broken cores: bit
 * per core quadrant that has broken off (LegionCorePart_Update); anim
 * palette: palette used by the palette animations (corpses and fragments
 * copy it); body palette: the base palette. Intro (Legion_UpdateIntro - TU
 * aliases STAGE / POSE / CYCLE): intro stage, the step the corpses and core
 * parts wait for (6, 7 ...) and the number of corpses / parts that have
 * reached their place in the current step.
 */
struct LegionWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ union { s32 value; struct EwramData_EntityData *ptr; } data;
    /* 0x4FC */ s8 introStep;
    /* 0x4FD */ u8 introArrived;
    /* 0x4FE */ u8 brokenCores;
    /* 0x4FF */ u8 pad_4FF[0x500 - 0x4FF];
    /* 0x500 */ u8 circleDir;
    /* 0x501 */ u8 pad_501[0x502 - 0x501];
    /* 0x502 */ u8 retargetTimer;
    /* 0x503 */ u8 pad_503[0x504 - 0x503];
    /* 0x504 */ u8 animPalette;
    /* 0x505 */ u8 bodyPalette;
    /* 0x506 */ s8 flags;
    /* 0x507 */ u8 introStage;
};

/* Legion core parts (Legion_CreateCorePart) and their details
 * (LegionCorePart_CreateDetail): the entity they hang on (shell parts keep
 * their owner here too), their offset from it (16.16), the tile data passed
 * to the fragments they shed, their kind / quadrant and a tick counter.
 */
struct LegionPartWork {
    /* 0x4F4 */ struct EwramData_EntityData *owner;
    /* 0x4F8 */ s32 offsetX;
    /* 0x4FC */ u32 offsetY;
    /* 0x500 */ s32 tiles;
    /* 0x504 */ u8 kind;
    /* 0x505 */ u8 tick;
};

/* Legion corpses (Legion_CreateCorpse, LegionCorpse_Update - TU aliases
 * TARGET_X / TARGET_Y / FRAME): room x it wanders to, the target y of its
 * rise, and the animation frame index.
 */
struct LegionCorpseWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 targetX;
    /* 0x4FC */ u32 targetY;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ u8 frame;
};

/* Legion shell parts (Legion_CreateShellPart, LegionShellPart_Update - TU
 * aliases KIND / ANGLE / RADIUS; owner in ext.legionPart.owner): orbit
 * radius, which of the three shells (index into LegionState.shells) they
 * belong to, release delay, kind, orbit angle and the centre entity they
 * circle.
 */
struct LegionShellWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 radius;
    /* 0x4FC */ s8 index;
    /* 0x4FD */ u8 pad_4FD[0x500 - 0x4FD];
    /* 0x500 */ u8 delay;
    /* 0x501 */ s8 kind;
    /* 0x502 */ s16 angle;
    /* 0x504 */ struct EwramData_EntityData *center;
};

/* Headhunter main entity: intro step (Headhunter AI TU); form phase (defeat
 * in phases 1 and 2 starts a transformation; the parts wait for phase KIND +
 * 1 - code_08083A40.c TU alias PHASE); hit react (set by
 * Headhunter_ApplyResolvedDamage when a hit should interrupt the current
 * action); flags (cleared every update; 1 = no contact damage this frame, 2
 * = show the forced frame instead of the animation); forced frame; graphics
 * = ObjGfx slot of its sprite sheet, shared by all parts and effects it
 * spawns; palettes 0..3 = the four palette variants of sHeadhunterPalette
 * (body forms and parts; 3 = effects).
 */
struct HeadhunterWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 introStep;
    /* 0x4F9 */ u8 pad_4F9[0x4FC - 0x4F9];
    /* 0x4FC */ s8 formPhase;
    /* 0x4FD */ u8 hitReact;
    /* 0x4FE */ u8 flags;
    /* 0x4FF */ u8 forcedFrame;
    /* 0x500 */ s32 graphics;
    /* 0x504 */ u8 palette0;
    /* 0x505 */ u8 palette1;
    /* 0x506 */ s8 palette2;
    /* 0x507 */ u8 palette3;
};

/* Headhunter effects, shots and parts (lightning, waves, orbiting particles
 * ...): the Headhunter whose flag bit 0 cancels them (TU alias OWNER_DONE
 * reads its flags), their first sprite frame (animation frame = base +
 * random) and the shared ObjGfx slot (copied from ext.headhunter.graphics).
 */
struct HeadhunterEffectWork {
    /* 0x4F4 */ struct EwramData_EntityData *source;
    /* 0x4F8 */ u8 pad_4F8[0x4FF - 0x4F8];
    /* 0x4FF */ u8 frame;
    /* 0x500 */ s32 graphics;
};

/* Headhunter orbiting particles: camera position of the last frame (to stay
 * room-fixed).
 */
struct HeadhunterOrbitWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 cameraX;
    /* 0x500 */ s32 cameraY;
};

/* Headhunter extending whip attack (Headhunter_SpawnExtendingAttack,
 * HeadhunterAttackTip_Update - TU aliases BASE_X / BASE_Y / OFFSET_X /
 * OFFSET_Y): the tip's 16.16 offset from the whip base and the room-space
 * base.
 */
struct HeadhunterWhipWork {
    /* 0x4F4 */ s32 offsetY;
    /* 0x4F8 */ u8 pad_4F8[0x4FC - 0x4F8];
    /* 0x4FC */ u32 baseX;
    /* 0x500 */ s32 baseY;
    /* 0x504 */ s32 offsetX;
};

/* Headhunter whip segments (Headhunter_CreateAttackSegment,
 * HeadhunterAttackSegment_Update): the tip and their index (1..9, placed at
 * index / 10 of the offset).
 */
struct HeadhunterWhipSegmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *tip;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ u8 index;
};

/* Headhunter transformation children (code_08083A40.c - TU alias KIND): kind
 * (transformation part 2..0, slash arc (1 / 2 drift with its facing), burst
 * corner 0..3); slash arcs keep the Headhunter in owner.
 */
struct HeadhunterChildWork {
    /* 0x4F4 */ struct EwramData_EntityData *owner;
    /* 0x4F8 */ u8 pad_4F8[0x504 - 0x4F8];
    /* 0x504 */ u8 kind;
};

/* Headhunter transformation parts (TU aliases COUNT / TARGET_X / TARGET_Y;
 * kind in ext.headhunterChild.kind): the room-space point they fly to and
 * the pass count.
 */
struct HeadhunterPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 targetX;
    /* 0x500 */ s32 targetY;
    /* 0x504 */ u8 pad_504[0x505 - 0x504];
    /* 0x505 */ u8 count;
};

/* Event scene 30 transformation controller
 * (EventScene30_UpdateTransformation and the unreferenced
 * EventScene30_UpdateUnusedZoom; TU aliases CENTER_X / CENTER_Y / SCALE_X /
 * TIMER / JITTER_Y / PHASE / INIT / ANGLE / SCALE_Y / SWEEP): BG2 affine
 * angle (high byte), phase and its first-frame flag, the scanline sweep
 * counter of the split zoom, the scale (the halo and centre effect use it as
 * their radius) and the vertical scale, the screen centre, the step timer
 * the effects watch (e.g. 263, 255, 160, 0), the horizontal jitter (read as
 * an s8; the vertical jitter is the byte above it, EVENT30_JITTER_Y in
 * event_scene30.h) and the texture y (high byte) of the split halves of the
 * unused zoom.
 */
struct Event30Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 angle;
    /* 0x4F9 */ u8 phase;
    /* 0x4FA */ u8 phaseInit;
    /* 0x4FB */ u8 pad_4FB[0x4FC - 0x4FB];
    /* 0x4FC */ s16 sweep;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s16 scale;
    /* 0x502 */ s16 scaleY;
    /* 0x504 */ s16 centerX;
    /* 0x506 */ s16 centerY;
    /* 0x508 */ s16 timer;
    /* 0x50A */ s16 jitterX;
    /* 0x50C */ u8 pad_50C[0x50E - 0x50C];
    /* 0x50E */ s16 zoomTexY;
};

/* Event scene 30 effect sprites (halo, centre effect, orbit ring; their
 * affine rotation angle is the entity field unk_51C): affine scale and the
 * spin step of the angle (centre and spinning effects).
 */
struct Event30EffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 scale;
    /* 0x504 */ u8 pad_504[0x50A - 0x504];
    /* 0x50A */ s16 spin;
};

/* Event scene 30 orbit ring effect (EventScene30_SpawnOrbitRing,
 * EventScene30OrbitRing_Update): index of its spawn point, orbit centre,
 * orbit angle, ring size (from the spawn point's scale), x / y orbit radius
 * and the outward spread counter.
 */
struct Event30RingWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FB - 0x4F4];
    /* 0x4FB */ u8 index;
    /* 0x4FC */ u8 pad_4FC[0x504 - 0x4FC];
    /* 0x504 */ s16 centerX;
    /* 0x506 */ s16 centerY;
    /* 0x508 */ s16 angle;
    /* 0x50A */ s16 size;
    /* 0x50C */ u8 radiusX;
    /* 0x50D */ u8 radiusY;
    /* 0x50E */ s16 spread;
};

/* Event scene 30 fragments (EventScene30_SpawnOrbitFragments,
 * *Fragment_Update): orbit direction (index & 1), fragment index and sway /
 * orbit phase.
 */
struct Event30FragmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FA - 0x4F4];
    /* 0x4FA */ u8 dir;
    /* 0x4FB */ u8 index;
    /* 0x4FC */ s16 phase;
};

/* Event scene 30 driver (EventScene30_Update): phase of the player's
 * floating sway during the transformation (Sin of it is passed to the
 * scripted animation).
 */
struct Event30DriverWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 floatPhase;
};

/* Balore body: eye / form animation index (4 at first; parts pick their
 * animation offsets with it), palette slot, the ObjGfx slot of its sprite
 * sheet (shared by all parts; also read through the byte view of its slot),
 * the command for the hands (state they switch to), rotation angle (read by
 * the overlay), flags (1 = hit, 2 = hurt reaction, 4 = dying, 8 / 0x10 =
 * left / right hand finished, 0x20) and affine scale (1.0, shrinks while
 * dying; the eye scales its aim offset with it).
 */
struct BaloreWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 eyeAnim;
    /* 0x4F7 */ u8 pad_4F7[0x4F8 - 0x4F7];
    /* 0x4F8 */ s8 palette;
    /* 0x4F9 */ u8 pad_4F9[0x4FB - 0x4F9];
    /* 0x4FB */ u8 gfx;
    /* 0x4FC */ s8 handCommand;
    /* 0x4FD */ u8 pad_4FD[0x500 - 0x4FD];
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 flags;
    /* 0x504 */ s32 scale;
};

/* Balore parts (BalorePart_Update) and hands: pointer to the body; parts:
 * animation base (0x12 / 0x1B while flashing).
 */
struct BalorePartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *body;
    /* 0x500 */ u8 anim;
};

/* Balore hands' hit boxes: pointer to their hand. */
struct BaloreHitBoxWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *hand;
};

/* Balore hands (body pointer in ext.balorePart.body): arm length (16.16, the
 * hit box sits at its end), arm angle, the horizontal scale of the stretched
 * arm sprite, the room position they return to (set by Balore_CreateHand)
 * and the heap block of their Entity_SteerToPoint flight parameters.
 */
struct BaloreHandWork {
    /* 0x4F4 */ s32 armLength;
    /* 0x4F8 */ u8 pad_4F8[0x500 - 0x4F8];
    /* 0x500 */ s16 armAngle;
    /* 0x502 */ u8 pad_502[0x504 - 0x502];
    /* 0x504 */ s32 armScale;
    /* 0x508 */ s16 homeX;
    /* 0x50A */ s16 homeY;
    /* 0x50C */ s32 steer;
};

/* Balore affine effect (BaloreAffineEffect_Update): angle, spin step and
 * scale.
 */
struct BaloreEffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 spin;
    /* 0x504 */ s32 scale;
};

/* Balore intro pieces (Balore_CreateIntro, BaloreIntroPiece_Update): the
 * intro entity, orbit angle, the shrinking orbit radius and the piece index
 * (start angle, orbit speed table index).
 */
struct BalorePieceWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *owner;
    /* 0x500 */ s16 angle;
    /* 0x502 */ s16 radius;
    /* 0x504 */ u8 index;
};

/* Balore overlay (BaloreOverlay_Update, follows ENTITY_OWNER = the body):
 * mirror flag (copies the flipped half of the body frame, swaps the layer,
 * flips the affine matrix), template info bytes it was set up from, and the
 * three heap blocks of its private one-frame sprite sheet: sheet, frame and
 * part list.
 */
struct BaloreOverlayWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F7 - 0x4F4];
    /* 0x4F7 */ u8 mirror;
    /* 0x4F8 */ s32 info;
    /* 0x4FC */ u32 sheet;
    /* 0x500 */ s32 frame;
    /* 0x504 */ s32 parts;
};

/* Balore emitter (BaloreEmitter_Draw): 16.16 x step between its 8 drawn
 * copies (the y step is outside the work area, BALORE_EMITTER_STEP_Y).
 */
struct BaloreEmitterWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ s32 stepX;
};

/* Save point (Object 0x1C, Object1CCreate / Object1CUpdate): yes selected =
 * cursor of the save prompt (1 = Yes; A saves only then); affine slot of the
 * growing / shrinking sprite and its scale; graphics = ObjGfx slot of
 * sSaveWarpRoomSpriteSheet, shared with its parts; frame base = first sprite
 * frame of the parts (they add 1..3); flags: 2 = the game was saved.
 */
struct SavePointWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FA - 0x4F4];
    /* 0x4FA */ u8 yesSelected;
    /* 0x4FB */ u8 affine;
    /* 0x4FC */ s16 graphics;
    /* 0x4FE */ s16 frameBase;
    /* 0x500 */ s32 scale;
    /* 0x504 */ u8 pad_504[0x508 - 0x504];
    /* 0x508 */ s16 flags;
};

/* Save point prompt (SavePoint_StepPromptFade): fade level 0..0x10000. */
struct SavePointPromptWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 fade;
};

/* Save point parts (SavePoint_SpawnAnimParts / SavePoint_SpawnOrbitLights):
 * the save point.
 */
struct SavePointPartWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *owner;
};

/* Save point orbit lights (SavePoint_UpdateOrbitLight; owner in
 * ext.savePointPart.owner): light index 0..7 (angle base index << 13), 16.16
 * radius (its integer half is also read as radius px,
 * SAVE_POINT_LIGHT_RADIUS_PX), and the phase: the radius growth counter
 * while spreading, then the angle offset while circling in.
 */
struct SavePointLightWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ u8 index;
    /* 0x505 */ u8 pad_505[0x508 - 0x505];
    /* 0x508 */ s32 radius;
    /* 0x50C */ s32 phase;
};

/* Chaos core: active orbs (attack request / orbs currently attacking), a
 * random 0..63 stored when the fight starts (never read), busy (an orb
 * attack is running), broken orbs (bit per destroyed orb), attack timers
 * 0..2 (random countdowns after which the core asks an orb to attack) and
 * data = address of its heap block (struct ChaosData, struct Chaos2Data in
 * the second form; read as a pointer by its children: the graphics / palette
 * block).
 */
struct ChaosWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F9 - 0x4F4];
    /* 0x4F9 */ u8 activeOrbs;
    /* 0x4FA */ u8 unusedRandom;
    /* 0x4FB */ u8 busy;
    /* 0x4FC */ s8 brokenOrbs;
    /* 0x4FD */ u8 attackTimer0;
    /* 0x4FE */ u8 attackTimer1;
    /* 0x4FF */ u8 attackTimer2;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ union { s32 value; struct EwramData_EntityData *ptr; } data;
};

/* Chaos second form core (data in ext.chaos.data): defeat flag set by its
 * affine effect when it has risen (written there as bytes[1]; the defeat
 * sequence Chaos2_UpdateDefeat waits for it), live parts (4 at first,
 * lowered by Chaos2Part_OnHit) and its base defense (Chaos2_OnHit uses parts
 * * 20 + base).
 */
struct Chaos2Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F9 - 0x4F4];
    /* 0x4F9 */ u8 defeatReady;
    /* 0x4FA */ u8 pad_4FA[0x500 - 0x4FA];
    /* 0x500 */ u8 partCount;
    /* 0x501 */ s8 baseDefense;
};

/* Chaos parts, orbs, shots and effects (both forms) point to the entity that
 * created them (the core, or the part / orb for their own effects).
 */
struct ChaosChildWork {
    /* 0x4F4 */ struct EwramData_EntityData *link;
};

/* Chaos second form parts: hit flash frames (30 after a hit; TU alias
 * FLASH).
 */
struct Chaos2PartWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s16 flash;
};

/* Chaos orbs (Chaos_CreateOrb, ChaosOrb_Update - TU aliases OFS_X / OFS_Y /
 * KIND): orbit angle offset (added to the background rotation), offset from
 * the core (pixels), kind (the soul type; its marks, halos and auras keep
 * the kind of their orb - TU aliases KIND / IDX) and the flag the soul shot
 * sets when its animation has ended (the orb reappears).
 */
struct ChaosOrbWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s16 offsetX;
    /* 0x502 */ s16 offsetY;
    /* 0x504 */ u8 pad_504[0x506 - 0x504];
    /* 0x506 */ s8 kind;
    /* 0x507 */ u8 shotDone;
};

/* Chaos shots, sparks and seeds of both forms (ChaosSeedShot_*,
 * ChaosSpark_*, ChaosSoulShot_Update, Chaos2Spark_Update ... - TU aliases
 * OFFSET_X / OFFSET_Y): 16.16 offset from the entity they follow (their orb
 * or the enemy target); sparks: grabbed (set by ChaosSpark_OnContact when it
 * grabs the player); seed / soul shots: reflected flag (set when the player
 * hits them back).
 */
struct ChaosShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 offsetX;
    /* 0x4FC */ u32 offsetY;
    /* 0x500 */ u8 grabbed;
    /* 0x501 */ u8 pad_501[0x504 - 0x501];
    /* 0x504 */ u8 reflected;
};

/* Chaos soul shot: its spread (same bytes as ext.chaosShot.reflected). */
struct ChaosSoulShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s32 spread;
};

/* Chaos second-form snake (Chaos2_CreateSnake, Chaos2SnakeHead_Update,
 * Chaos2SnakeSegment_Update): turn speed, waypoint set (mode 0..3), camera x
 * / y low bytes of the last frame (compensates scrolling), segment index
 * (0..7, 7 = tail), waypoint number, waypoint target x / y, affine angle,
 * the head's move timer and the heap block with the head's 120-entry
 * position history (6 bytes per entry).
 */
struct ChaosSnakeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 turn;
    /* 0x4FA */ u8 pad_4FA[0x4FB - 0x4FA];
    /* 0x4FB */ u8 mode;
    /* 0x4FC */ s8 cameraX;
    /* 0x4FD */ u8 cameraY;
    /* 0x4FE */ u8 index;
    /* 0x4FF */ u8 point;
    /* 0x500 */ s16 targetX;
    /* 0x502 */ s16 targetY;
    /* 0x504 */ s16 angle;
    /* 0x506 */ s16 timer;
    /* 0x508 */ s32 trail;
};

/* Chaos rotating rings (Chaos_CreateOuterRing / InnerRing, *Ring_Update - TU
 * aliases SPIN / ORBIT / SPIN_BASE / ORBIT_BASE / CENTER_X / CENTER_Y /
 * RADIUS): running spin and orbit angles, their fixed offsets per ring,
 * orbit centre (pixels) and radius.
 */
struct ChaosRingWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 spin;
    /* 0x4FA */ s16 orbit;
    /* 0x4FC */ s16 spinOffset;
    /* 0x4FE */ s16 orbitOffset;
    /* 0x500 */ s16 centerX;
    /* 0x502 */ s16 centerY;
    /* 0x504 */ s16 radius;
};

/* Chaos second-form spinning debris (Chaos2SpinningDebris_Update): affine
 * angle, advanced by accelX.
 */
struct ChaosDebrisWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
};

/* Player entity (entity slot 0, Soma and Julius code; the link partner
 * entity keeps the same action bits): action flags = the player state bit
 * field (bit 1 airborne, 0x10 double jump / ability jump used, 0x20 weapon
 * attack, 0x400 crouching, 0x1000 standing on a solid floor, 0x2000/0x4000
 * on a slope, 0x80000 an attack connected this frame, 0x200000 the requested
 * animation has been started, ...; weapons, souls and enemies read it
 * through the owner pointer); air time = frames since leaving the floor,
 * counted up to 16 (a jump sets it to 16; a ground jump is still allowed
 * while it is <= 3); drop-through timer = set to 16 (32) by down + jump,
 * while nonzero the floor probes are skipped and no jump starts; wall probes
 * = address of the list of wall probe offsets used by the horizontal
 * collision (chosen per state); on stairs = a stair tile was found in front
 * of the feet; slope level = steepest slope class (collision flags >> 6)
 * under the feet, SlopeSlowdown divides velX by it; invulnerable timer =
 * nonzero blocks Player_ApplyContactDamage (souls and damage set it, the
 * updates count it down); requested anim = animation index started at the
 * end of the frame (0xFF = none); frame counter = incremented once per
 * player update; draw offset = signed pixel offset added to the position
 * while Player_DrawWithOffset draws the sprite; anim palette = palette index
 * last loaded for the current animation (sSomaAnimPalettes[anim] is compared
 * with it before ObjPalette_LoadRange); transform form = form index of the
 * transformation souls (Skill_Devil_Manticore_Curly_Use; picks the dash
 * animations of Soma_UpdateTransformationDash); weapon slot = index into
 * playerState.weaponEntities and the attacker info of the weapon this entity
 * attacks with (0 = player, 1 = link partner); floor kind = 0xFF = none yet
 * this frame, else the floor/slope kind found by the collision probes (reset
 * to 0xFF after use); up hold timer = frames up has been held while standing
 * (look-up animation after 4).
 */
struct PlayerWork {
    /* 0x4F4 */ s32 actionFlags;
    /* 0x4F8 */ u16 airTime;
    /* 0x4FA */ s16 dropThroughTimer;
    /* 0x4FC */ u32 wallProbes;
    /* 0x500 */ u8 onStairs;
    /* 0x501 */ s8 slopeLevel;
    /* 0x502 */ u8 invulnerableTimer;
    /* 0x503 */ u8 pad_503[0x504 - 0x503];
    /* 0x504 */ s16 requestedAnim;
    /* 0x506 */ s16 frameCounter;
    /* 0x508 */ u8 drawOffsetX;
    /* 0x509 */ u8 drawOffsetY;
    /* 0x50A */ u8 animPalette;
    /* 0x50B */ u8 transformForm;
    /* 0x50C */ u8 weaponSlot;
    /* 0x50D */ u8 floorKind;
    /* 0x50E */ u8 upHoldTimer;
};

/* Solid objects (Player_RegisterSolidObject, playerState.solidObject): their
 * callback table (first word = flags, Player_GetSolidObjectFlags), stored
 * through the entity pointer view and read through the word pointer view (TU
 * alias FUNCS of the player TUs).
 */
struct SolidObjectWork {
    /* 0x4F4 */ union { s32 *words; struct EwramData_EntityData *table; } callbacks;
};

/* Ghost spirit / Tiny Devil homing shots and the homing soul projectile
 * (HomingSoulProjectile_Update): target and 16.16 speed (accelerating; the
 * homing soul keeps its speed outside the work area).
 * Homing soul projectile: a halfword cleared at launch, never read.
 */
struct SkillHomingWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ struct EwramData_EntityData *target;
    /* 0x508 */ s32 speed;
    /* 0x50C */ u8 pad_50C[0x50E - 0x50C];
    /* 0x50E */ s16 unused;
};

/* Death's sickle soul (SkillDeathSickleUpdate): flight speed and scale
 * (grows to 1.0).
 */
struct SkillDeathSickleWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 speed;
    /* 0x504 */ u8 pad_504[0x50C - 0x504];
    /* 0x50C */ s32 scale;
};

/* Alastor sword familiar (SkillAlastorUse): affine scale. */
struct SkillAlastorWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ s32 scale;
};

/* Red Minotaur axe swing (SkillRedMinotaurUse, read by its hit box): scale
 * and swing acceleration.
 */
struct SkillRedMinotaurWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 scale;
    /* 0x50C */ s32 swingAccel;
};

/* Winged Skeleton spear (SkillWingedSkeletonUse): the spear tip offset that
 * its hit box follows.
 */
struct SkillWingedSkeletonWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 tipX;
    /* 0x50C */ s32 tipY;
};

/* Gladiator spinning blade (SkillGladiatorUse): walk speed. */
struct SkillGladiatorWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 speed;
};

/* Chronomage clock (SkillChronomageUse): affine scale, a second ramp that is
 * updated like the scale but never used, and the palette inversion level.
 */
struct SkillChronomageWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s32 scale;
    /* 0x508 */ s32 ramp;
    /* 0x50C */ s32 invert;
};

/* Thrown objects of the Skeleton-family red souls (bone, grenade, rock...,
 * Skill_Skeleton_..._Use, BoneSoulInfo table): 16.16 throw speed (used as
 * the walk speed of rolling objects).
 */
struct SkillThrowWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 speed;
};

/* Witch / Giant Ghost soul (Skill_Witch_GiantGhost_Use): affine scale. */
struct SkillWitchWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ s32 scale;
};

/* Boomerang / Axe Armor axe (SkillBoomerangProjectileUpdate,
 * SkillAxeArmorUse): spin direction (+1 / -1) and the spin counter (affine
 * angle << 7 / << 11).
 */
struct SkillBoomerangWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 spinDir;
    /* 0x50A */ u8 pad_50A[0x50E - 0x50A];
    /* 0x50E */ s16 spin;
};

/* Ukoback fireball soul (SkillUkobackUse): affine scale. */
struct SkillUkobackWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 scale;
};

/* Disc Armor soul (SkillDiscArmorUse, its scaled sprite reads the parent's
 * angle): orbit radius and affine scale (pulsing).
 */
struct SkillDiscArmorWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 radius;
    /* 0x50C */ s32 scale;
};

/* Cockatrice soul (SkillCockatriceUse / Draw): beam y scale. */
struct SkillCockatriceWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 scaleY;
};

/* Blue Crow soul (SkillBlueCrowUse): orbit radius. */
struct SkillBlueCrowWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 radius;
};

/* Slime soul (SkillSlimeUse, SkillSlimeMoveAndBounce): lifetime counter. */
struct SkillSlimeWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 timer;
};

/* Creaking Skull soul (SkillCreakingSkullUse): affine scale. */
struct SkillCreakingSkullWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ s32 scale;
};

/* Bone Pillar soul (SkillBonePillarUse): index 0..2 of the next launched
 * bone.
 */
struct SkillBonePillarWork {
    /* 0x4F4 */ u8 pad_4F4[0x50A - 0x4F4];
    /* 0x50A */ s16 shot;
};

/* Bone souls (Skill_Skeleton_..._Use): hit flag of the bone projectile, set
 * by BoneSoul_OnPlayerContact; with info flag 0x100 a hit bone shatters.
 */
struct SkillBoneWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ s16 hit;
};

/* Ripper soul: spin (affine angle in 1/20 turns). */
struct SkillRipperWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 spin;
};

/* Nemesis soul: lifetime (300 frames). */
struct SkillNemesisWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 timer;
};

/* Siren soul: sway phase (Sin of it is the vertical speed). */
struct SkillSirenWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 phase;
};

/* Une / Biphron souls: walk speed (Une bounces back with half of it). */
struct SkillWalkerWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 speed;
};

/* Nightmare soul: horizontal affine scale. */
struct SkillNightmareWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 scaleX;
};

/* Kyoma Demon soul: horizontal affine scale. */
struct SkillKyomaDemonWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s32 scaleX;
};

/* Kyoma Demon soul: contact-immunity wait (180 frames; same bytes as the
 * scale).
 */
struct SkillKyomaDemonWaitWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 timer;
};

/* Alura Une / Shadow Knight / Big Golem souls: affine x scale growing to 1.0
 * (the y scale is outside the work area).
 */
struct SkillGrowScaleWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ s32 scaleX;
};

/* Catoblepas soul: shot index (0..2, launch speed table). */
struct SkillCatoblepasWork {
    /* 0x4F4 */ u8 pad_4F4[0x50A - 0x4F4];
    /* 0x50A */ s16 shotIndex;
};

/* Object 0x0D (swinging object hanging from the ceiling, Object0DUpdate):
 * affine angle (clamped to +-0x1000), angular speed and the swing amplitude
 * (0x100 after an impulse, loses 0x20 per swing).
 */
struct Object0DWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 angle;
    /* 0x500 */ s32 speed;
    /* 0x504 */ s32 amplitude;
};

/* Object 0x16 (affine zoom-out cutscene object, Object16Update): zoom timer
 * (0x100 frames), BG affine scale, rotation angle and its accelerating step.
 */
struct Object16Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 timer;
    /* 0x4FC */ u8 pad_4FC[0x500 - 0x4FC];
    /* 0x500 */ s32 scale;
    /* 0x504 */ s32 angle;
    /* 0x508 */ s32 spin;
};

/* Object 0x37 (Object37_*): affine angle (0..0x3000) and the closed flag (1
 * while its collision blocks are set).
 */
struct Object37Work {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 angle;
    /* 0x500 */ s32 closed;
};

/* Object 0x37 child (Object37Child_Update): affine angle, turns between 0
 * and -0x2000.
 */
struct Object37ChildWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 angle;
};

/* Object 0x14 (rolling affine sprite, Object14Variant0/1/2_Update): flags
 * (variant 1 is pushed by the player: bit 0, set by
 * Object14_WallLeft/RightCorrection), room pixel position and the position
 * of the previous frame, affine angle; variant 0 also grows its scale.
 */
struct Object14Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 flags;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ s16 roomX;
    /* 0x4FE */ s16 roomY;
    /* 0x500 */ s16 prevX;
    /* 0x502 */ s16 prevY;
    /* 0x504 */ s16 angle;
    /* 0x506 */ u8 pad_506[0x508 - 0x506];
    /* 0x508 */ s32 scale;
};

/* Object 0x14 variant 2 (Object14Variant2_Update; room / previous position
 * in ext.object14): swing angle and its random speed.
 */
struct Object14SwingWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 angle;
    /* 0x50A */ s16 speed;
};

/* Object 0x31 (Object31_UpdateState): start delay, then an affine angle
 * turning with a speed that accelerates by the step (4) up to 0x60.
 */
struct Object31Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 delay;
    /* 0x4FC */ u32 angle;
    /* 0x500 */ s32 speed;
    /* 0x504 */ s32 step;
};

/* Object 0x35 (floor switch, Object35_*): 16.16 home position (its integer
 * halves are also read directly, OBJECT35_HOME_PIXEL_X / _Y), frames the
 * player has stood on it, and the pressed flag (drawn 0x30 px lower).
 */
struct Object35Work {
    /* 0x4F4 */ s32 homeX;
    /* 0x4F8 */ s32 homeY;
    /* 0x4FC */ u32 standFrames;
    /* 0x500 */ s32 pressed;
};

/* Object 0x35 child: the floor switch. */
struct Object35ChildWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ struct EwramData_EntityData *owner;
};

/* Object 0x36 (rotating BG-affine platform, Object36*): rotation angle,
 * radius (half the shape width), the angle step and the BG affine scale.
 */
struct Object36Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ s16 radius;
    /* 0x4FE */ s16 spin;
    /* 0x500 */ s16 scale;
};

/* Object 0x30 falling leaf (Object30_SpawnFallingLeaf, Object30Leaf_Update):
 * kind 0..2 (animation, fall speed, parallax) and the BG 1 x scroll of the
 * previous frame (for the parallax).
 */
struct Object30LeafWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 kind;
    /* 0x4FC */ u32 scrollX;
};

/* Object 0x19 (Object19Create / Object19Update): affine angle and its step,
 * time left in the current path step of sObject19Steps (16.16 frames; the
 * first step starts at half its time).
 */
struct Object19Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 angle;
    /* 0x4FA */ s16 spin;
    /* 0x4FC */ u32 stepTime;
};

/* Object 0x15 (Object15Update, its orbiters read the owner): orbit angle
 * (+0x40 per frame) and the 16.16 orbit radius (shrinks from 0x100000).
 */
struct Object15Work {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s32 radius;
};

/* Object 0x08 (breakable block, Object08Update) and its dust child
 * (BreakableBlock_SpawnDust): block kind / orientation byte (copied to the
 * child, selects the room flag and the dust spread).
 */
struct Object08Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 kind;
};

/* Object 0x08 dust child: the dust ObjGfx slot. */
struct Object08DustWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 gfx;
};

/* Object 0x1B (conveyor, Object1BUpdate): scroll position and the colour of
 * the current frame.
 */
struct Object1BWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 scroll;
    /* 0x4FC */ s8 color;
};

/* Object 0x25 spawner (Object25Update): live children. */
struct Object25SpawnerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 childCount;
};

/* Object 0x25 children: the spawner (decremented when the child is deleted). */
struct Object25Work {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ struct EwramData_EntityData *spawner;
};

/* Elevator (object 0x17, Object17Update): frames up / down has been held on
 * it (starts moving after 3).
 */
struct Object17Work {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 holdFrames;
};

/* Warp room (object 0x1F, WarpRoom_Update): bottom line of window 0 (0x74 on
 * arrival, closes to 0x24).
 * ObjGfx slot of sWarpRoomGfx: stored, never read.
 */
struct WarpRoomWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 windowBottom;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ s16 unusedGfx;
};

/* Object 0x26 (Object26Create / _UpdateState): frame clocks of its two
 * traps, counted while the target is in its area; object 0x27 fires when the
 * first reaches 0x78 (starts at 0x1E), object 0x28 when the second does
 * (starts at 0x5A); each trap resets its clock.
 */
struct Object26Work {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 clock27;
    /* 0x4FE */ s16 clock28;
};

/* Warp room gate parts (WarpRoom_UpdateGatePart - TU aliases DIST / SPEED /
 * ACCEL / POS / STEP / HOMEX / HOMEY): travelled distance, acceleration and
 * speed of the opening, home x / y, position / step of the part.
 */
struct WarpGateWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 dist;
    /* 0x4FC */ s16 accel;
    /* 0x4FE */ s16 speed;
    /* 0x500 */ s16 homeX;
    /* 0x502 */ s16 homeY;
    /* 0x504 */ s16 pos;
    /* 0x506 */ s16 step;
};

/* Object 0x1D (warp room side wall, Object1DCreate): side of the room exit
 * (1 when the exit directions of the room sum to +1, i.e. not flipped).
 */
struct Object1DWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 exitRight;
};

/* Object 0x22 (platform carrying the player along its levels,
 * Object22_UpdateState): frames without the player on it (returns after 60)
 * and the flag byte (bit 0: no longer queued as a platform).
 */
struct Object22Work {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ u8 emptyTimer;
    /* 0x501 */ u8 pad_501[0x507 - 0x501];
    /* 0x507 */ u8 flags;
};

/* Object 0x22 falling part: the platform. */
struct Object22PartWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *owner;
};

/* Object 0x07 (BG decoration): the palette-animation variant clears frame
 * and tick (BG_PALETTE_ANIM_*, TU code_0804F690.c) as one word; its parallax
 * sprites keep their frame.
 */
struct Object07Work {
    /* 0x4F4 */ s32 paletteAnim;
    /* 0x4F8 */ u8 pad_4F8[0x4FC - 0x4F8];
    /* 0x4FC */ s8 spriteFrame;
};

/* Object 0x09: collision mode of its definition (passed to
 * GameFlag_FindRoomCoordinate).
 */
struct Object09Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 collisionMode;
};

/* Object 0x10 (Boss Rush reward): the item given (chosen from the time). */
struct Object10Work {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 rewardItem;
};

/* Boss Rush timer (BossRushTimer_Update, entity slot 26): frames elapsed
 * (stops at 0x34BBD), read by the result screen and the reward.
 */
struct BossRushTimerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 time;
};

/* Object 0x1F (exit with a fade): frames until the room transition. */
struct Object1FWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 fadeTimer;
};

/* Title screen menu entity (entity slot 0x14): cursor within the visible
 * entries, number of visible entries, confirmed (input locked after
 * Start/A), first entry index (0 = main entries, 3 = extra entries), number
 * of extra entries (2 with systemFlags bit 1), slide active / slide phase (1
 * = slide out, 2 = slide back) and the 16.16 slide offset of alternate rows.
 */
struct TitleMenuWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 cursor;
    /* 0x4FD */ u8 count;
    /* 0x4FE */ u8 confirmed;
    /* 0x4FF */ u8 pad_4FF[0x500 - 0x4FF];
    /* 0x500 */ u8 first;
    /* 0x501 */ s8 extraCount;
    /* 0x502 */ u8 sliding;
    /* 0x503 */ u8 slidePhase;
    /* 0x504 */ s32 slideOffset;
};

/* Main menu / save select entity (GameModeMainMenuUpdate, code_08005894.c):
 * slot empty (1 when the shown slot cannot be loaded / copied), saved slots
 * (bit per slot whose header has a play time, MainMenu_InitEntity; 0 = no
 * saved game, the choice stays on New Game), copy source / destination slots
 * of Data Copy, cursor (0-5 = save slots, 6+ = menu items, or the name-entry
 * grid cursor), name cursor (character position 0-7 in the name entry),
 * confirm open / cursor (the yes/no box of the game start / copy / delete
 * flow), base frame (animFrame saved at init), frame timer and step of the
 * eight-step BG palette cycle sMainMenuPaletteCycleSteps, blink timer /
 * phase of the confirmation marker.
 */
struct MainMenuWork {
    /* 0x4F4 */ u8 slotEmpty;
    /* 0x4F5 */ u8 savedSlots;
    /* 0x4F6 */ u8 copySource;
    /* 0x4F7 */ u8 copyDest;
    /* 0x4F8 */ u8 pad_4F8[0x4F9 - 0x4F8];
    /* 0x4F9 */ u8 cursor;
    /* 0x4FA */ u8 pad_4FA[0x4FB - 0x4FA];
    /* 0x4FB */ u8 nameCursor;
    /* 0x4FC */ u8 pad_4FC[0x4FD - 0x4FC];
    /* 0x4FD */ u8 confirmOpen;
    /* 0x4FE */ u8 confirmCursor;
    /* 0x4FF */ u8 pad_4FF[0x500 - 0x4FF];
    /* 0x500 */ u8 baseFrame;
    /* 0x501 */ s8 paletteTimer;
    /* 0x502 */ u8 paletteStep;
    /* 0x503 */ u8 pad_503[0x504 - 0x503];
    /* 0x504 */ u8 blinkTimer;
    /* 0x505 */ u8 blinkPhase;
};

/* Title logo entity (entityData[0x12], TitleScreenLogo_*): background
 * palette blend timer (sTitlePaletteBlendSteps, TitleScreen_sub_08002FF8),
 * flags (1 = the approach is nearly done, start the palette fade-in; 2 =
 * fade-in done), the zoom step that shrinks the BG2 scale to 0x118 - step
 * after the approach (counts to 0x40), the approach distance that
 * GameModeTitleScreenUpdate reduces to 0 while the scale grows (0x108 -
 * distance / 0x8000), the amplitude of the BG1 h-blank wave
 * (TitleScreen_sub_08002F44) that fades out at the end of the intro, the
 * palette blend step and the wobble phase of the idle scale.
 */
struct TitleLogoWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 paletteTimer;
    /* 0x4FC */ s16 flags;
    /* 0x4FE */ s16 zoomStep;
    /* 0x500 */ s32 distance;
    /* 0x504 */ s32 waveAmplitude;
    /* 0x508 */ u8 paletteStep;
    /* 0x509 */ u8 pad_509[0x50A - 0x509];
    /* 0x50A */ s16 wobblePhase;
};

/* Title effect group (entityData[0x13], TitleScreen_CreateEffects): the fade
 * timer of the blend in front of the children and flags read by them (1 =
 * show the particles, 2 = start the spiral sprite, 4 = particles become
 * opaque).
 */
struct TitleEffectsWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 fadeTimer;
    /* 0x4FC */ s16 flags;
};

/* Title particles: 16.16 drift added to the position every frame and the two
 * sine phases of the wobble (advanced by velX / velY, amplitudes accelX /
 * accelY).
 */
struct TitleParticleWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 driftX;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ s32 driftY;
    /* 0x508 */ u8 pad_508[0x50C - 0x508];
    /* 0x50C */ s16 phaseX;
    /* 0x50E */ s16 phaseY;
};

/* Title spiral sprite: angle and radius around (0x7A, 0x1A), affine scale;
 * unused is cleared at init and never read.
 */
struct TitleSpiralWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 unused;
    /* 0x500 */ s32 radius;
    /* 0x504 */ s32 scale;
};

/* Title flash sprites (TitleScreen_SpawnFlash): lifetime (sine phase of the
 * blend fade, counts down by 0x180), rotation angle and spin per frame, size
 * (scale = size << 12), scale and the flag that leaves the blend alpha
 * alone.
 */
struct TitleFlashWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 life;
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 spin;
    /* 0x500 */ s32 size;
    /* 0x504 */ s32 scale;
    /* 0x508 */ u8 noBlend;
};

/* Intro cutscene shrinking / floating sprites: x / y sway phases (floating
 * sprite) and the affine scale.
 */
struct IntroSpriteWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 swayX;
    /* 0x4FC */ u32 swayY;
    /* 0x500 */ s32 scale;
};

/* Intro cutscene drifting sprite: fade timer (0x120 frames, sets the blend). */
struct IntroDriftWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 fade;
};

/* Intro cutscene text (IntroCutsceneText_PrintNextLine): next text row, the
 * position in the script and the initial BG0 vertical scroll (stored with
 * the register and never read).
 */
struct IntroTextWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F9 - 0x4F4];
    /* 0x4F9 */ u8 row;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ s16 pos;
    /* 0x4FE */ s16 startScroll;
};

/* Soul trade menu entity (code_080096AC.c): selected soul kind (list 0-2,
 * sSoulTradeListTextBases); partner-offer-shown flag; selected soul list
 * entry (halfword, and its low byte as read by SoulTrade_MoveListCursor);
 * link sync retry count and link wait counter of GameModeSoulTradeMenu
 * (counts to 6, once stores a link id); own offer kind / soul id and partner
 * offer kind / soul id (sent with SoulTrade_SetOffer /
 * SoulTrade_SetAcknowledge, printed by SoulTrade_UpdateOfferNames); text
 * entry currently printed in the message window (reprinted only when it
 * changes); cursor row inside the eight-row window and the window's first
 * row.
 */
struct SoulTradeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 listKind;
    /* 0x4F9 */ u8 partnerShown;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ union { s16 value; s8 low; } selected;
    /* 0x500 */ s16 syncCount;
    /* 0x502 */ s16 waitCounter;
    /* 0x504 */ u8 ownKind;
    /* 0x505 */ u8 ownSoul;
    /* 0x506 */ s8 partnerKind;
    /* 0x507 */ u8 partnerSoul;
    /* 0x508 */ s32 shownText;
    /* 0x50C */ s16 listRow;
    /* 0x50E */ s16 listTop;
};

/* Sound test menu text entity (GameModeSoundTestMenu): the entry it shows. */
struct SoundTestTextWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 entry;
};

/* Item icon entities (ItemIconEntity_Update, ItemIcon_AttachSprite / _Free):
 * the itemIconIds cache slot it uses, item id and palette.
 */
struct ItemIconWork {
    /* 0x4F4 */ u8 slot;
    /* 0x4F5 */ u8 pad_4F5[0x4F8 - 0x4F5];
    /* 0x4F8 */ s8 id;
    /* 0x4F9 */ u8 palette;
};

/* Player status icon (Player_SpawnStatusIcon, PlayerStatusIcon_Update):
 * index of the status effect it shows (its low byte picks the frame).
 */
struct StatusIconWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ union { s16 value; s8 low; } index;
};

/* Weapon attack entities (Weapon*_Update, created by
 * Weapon_CreateAttackEntity; Julius' whip strike by
 * Julius_CreateWhipAttackEntity): the attacking entity (player or link
 * partner) and the weapon that was equipped when the attack started (the
 * attack stops hitting when it changes). Heavy weapons (axes) also keep the
 * frame on which the attack ends, the swing angle of their affine sprite and
 * its speed per frame.
 */
struct WeaponWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *owner;
    /* 0x4FC */ s8 id;
    /* 0x4FD */ u8 endTime;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s32 swingAngle;
    /* 0x504 */ s32 swingSpeed;
};

/* Gun shot weapon attack (owner / id in ext.weapon): stop flag (set when it
 * hits a wall or an enemy; same bytes as the swing angle of heavy weapons).
 */
struct WeaponGunWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 stopped;
};

/* Whip chain entity (Whip_CreateChain; 8 WhipChainNode records from 0x4F4):
 * active flag (Whip_Activate/DeactivateChain), settle counter (frames the
 * anchor node has not moved; the node simulation stops at 120, the swing
 * helpers reset it) and its owner.
 * Nodes: the 8 WhipChainNode records (gEwramData->whipChainNodes).
 */
struct WhipChainWork {
    /* 0x4F4 */ struct WhipChainNode *nodes;
    /* 0x4F8 */ u8 pad_4F8[0x500 - 0x4F8];
    /* 0x500 */ s16 active;
    /* 0x502 */ s16 settle;
    /* 0x504 */ struct EwramData_EntityData *owner;
};

/* Whip hit segments created by Whip_CreateChain: index of the node they
 * follow.
 */
struct WhipSegmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ u8 node;
};

/* Julius sub-weapons: the cross and axe projectiles spin by adding the spin
 * speed to the rotation angle passed to ObjAffine_Set; the holy water bottle
 * and its flame walk with Entity_Walk at the stored speed.
 */
struct SubweaponWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 spinAngle;
    /* 0x50A */ s16 spinSpeed;
    /* 0x50C */ s32 walkSpeed;
};

/* Julius sub-weapon (code_080D1E5C.c): a sum accumulated every frame but
 * never read; the step is never written.
 */
struct SubweaponUnusedWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 step;
    /* 0x50A */ u8 pad_50A[0x50E - 0x50A];
    /* 0x50E */ s16 sum;
};

/* Julius Grand Cross: 16.16 scale of its affine sprite. */
struct GrandCrossWork {
    /* 0x4F4 */ u8 pad_4F4[0x50C - 0x4F4];
    /* 0x50C */ s32 scale;
};

/* Object 0x34 (swinging platform): swing angle, angular speed (+-0x100) and
 * the acceleration applied at the turning points (spawnParam1).
 */
struct Object34Work {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 angle;
    /* 0x4FE */ s16 speed;
    /* 0x500 */ s16 accel;
};

/* Object 0x34 parts (0, 1 chain links, 2 the platform): their index and the
 * pivot entity.
 */
struct Object34PartWork {
    /* 0x4F4 */ u8 pad_4F4[0x502 - 0x4F4];
    /* 0x502 */ u8 index;
    /* 0x503 */ u8 pad_503[0x504 - 0x503];
    /* 0x504 */ struct EwramData_EntityData *pivot;
};

/* Event scene 04 actor: position in the animation list sUnk_0852682C (0xFF
 * ends it; also read through the byte view), and a flag set to 4 when its
 * text starts (never read).
 */
struct Event04Work {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 animPos;
    /* 0x4F9 */ u8 pad_4F9[0x4FC - 0x4F9];
    /* 0x4FC */ u8 unusedFlag;
};

/* Animation viewer (debug): frame mode (0 = play animation scripts, 1 = step
 * single frames) and the selected frame.
 */
struct AnimViewerWork {
    /* 0x4F4 */ u8 pad_4F4[0x504 - 0x4F4];
    /* 0x504 */ u8 frameMode;
    /* 0x505 */ u8 frame;
};

/* Sprite debris (Effect_UpdateSpriteDebris): pixel offset of its ground
 * probe from the position.
 */
struct SpriteDebrisWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 probeX;
    /* 0x4FE */ s16 probeY;
};

/* Valkyrie spear attack (Valkyrie_UpdateAttack): frame counter of the
 * current attack step.
 */
struct ValkyrieWork {
    /* 0x4F4 */ s32 attackFrame;
};

/* Needles soul: draw copies on circles when set; nothing sets it
 * (SkillNeedlesUse leaves it 0).
 */
struct SkillNeedlesWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 spread;
};

/* Man Eater soul skill: frames since the bite started (ends after 0x1D). */
struct SkillManEaterWork {
    /* 0x4F4 */ u8 pad_4F4[0x508 - 0x4F4];
    /* 0x508 */ s16 frames;
};

/* Creaking Skull soul segments: 0x200000 stored at creation; never read. */
struct SkillCreakingSkullSegmentWork {
    /* 0x4F4 */ u8 pad_4F4[0x500 - 0x4F4];
    /* 0x500 */ s32 unused;
};

/* Debug test walkers (DebugLinkTestWalker_Update,
 * DebugTestWalkerUnused_Update): air state - 0 = on the ground, 1 = in the
 * air (jumping / falling), 2 = jump button pressed again while airborne.
 */
struct DebugWalkerWork {
    /* 0x4F4 */ s32 airState;
};

/* Number indicator: glyph slots (11 bytes from 0x4FC; slot 0 = sign / prefix
 * frame, slot 1 = first digit, see NumberSlot), the number of zero-padded
 * trailing digits; number watch (NumberWatch_Create / _Update): width of the
 * watched value in bits (8 / 16 / 32) and its address.
 */
struct NumberIndicatorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s8 prefix;
    /* 0x4FD */ u8 firstDigit;
    /* 0x4FE */ u8 pad_4FE[0x508 - 0x4FE];
    /* 0x508 */ u8 padDigits;
    /* 0x509 */ u8 watchBits;
    /* 0x50A */ u8 pad_50A[0x50C - 0x50A];
    /* 0x50C */ s32 watchAddress;
};

/* Soul pickup (flies to the player like the homing orb): flight angle, turn
 * rate per frame (starts at the stationary argument, grows to 0x4000) and
 * 16.16 speed (grows to 0x80000).
 */
struct SoulPickupWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 angle;
    /* 0x500 */ s32 turn;
    /* 0x504 */ s32 speed;
};

/* Soul container: the soul sprite entity it shows inside and the palette of
 * the soul variant.
 */
struct SoulContainerWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *soul;
    /* 0x500 */ u8 palette;
};

/* Homing orb (Effect_CreateHomingOrb / Effect_UpdateHomingOrb): the entity
 * it flies to, its flight angle, the turn rate per frame (grows to 0x4000),
 * the 16.16 speed (grows to 0x80000) and the y offset of the aim point.
 */
struct HomingOrbWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *target;
    /* 0x4FC */ u32 angle;
    /* 0x500 */ s32 turn;
    /* 0x504 */ s32 speed;
    /* 0x508 */ s32 targetDy;
};

/* Soul type selector (SoulTypeSelector_Update): a value turned with the spin
 * but never read (padding of its SoulTypeIcons view) and the signed icon
 * rotation step per frame (SoulTypeIcons.dir).
 */
struct SoulSelectorWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ s16 unusedSpin;
    /* 0x4F8 */ u8 pad_4F8[0x504 - 0x4F8];
    /* 0x504 */ s32 spin;
};

/* BG palette animation (BgPaletteAnim_Step): current frame of the cycle and
 * the frames shown of it (object 0x07 clears both as one word,
 * ext.object07.paletteAnim).
 */
struct BgPaletteAnimWork {
    /* 0x4F4 */ s16 frame;
    /* 0x4F6 */ s16 tick;
};

/* Warp room side light: animation frame (WarpRoom_UpdateSideLight reads it
 * as FRAME, s8).
 */
struct WarpRoomSideLightWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F6 - 0x4F4];
    /* 0x4F6 */ u8 frame;
};

/* Unused warp room part (WarpRoomUnusedPart_Update): affine angle, rise
 * frame, its affine slot and the scale (shrinks to 0).
 * Parent: the entity it was spawned by.
 */
struct WarpRoomUnusedPartWork {
    /* 0x4F4 */ s32 angle;
    /* 0x4F8 */ u16 riseFrame;
    /* 0x4FA */ u8 pad_4FA[0x4FB - 0x4FA];
    /* 0x4FB */ u8 affine;
    /* 0x4FC */ struct EwramData_EntityData *parent;
    /* 0x500 */ s32 scale;
};

/* Peeping Eye body: animation set, hit flag (PeepingEye_OnHit), animation
 * flags (bit 1 = eye open), frame, step index and clock, and the room-space
 * movement target.
 */
struct PeepingEyeBodyWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 animSet;
    /* 0x4F9 */ u8 hitFlag;
    /* 0x4FA */ u8 pad_4FA[0x4FC - 0x4FA];
    /* 0x4FC */ s8 animFlags;
    /* 0x4FD */ u8 animFrame;
    /* 0x4FE */ u8 animIndex;
    /* 0x4FF */ u8 animClock;
    /* 0x500 */ s32 targetX;
    /* 0x504 */ s32 targetY;
};

/* Peeping Eye chain links (owner eye in ext.peepingEye.owner): timer, link
 * index, angle, length, maximum length and the previous link.
 */
struct PeepingEyeLinkWork {
    /* 0x4F4 */ u8 timer;
    /* 0x4F5 */ u8 index;
    /* 0x4F6 */ s16 angle;
    /* 0x4F8 */ s32 length;
    /* 0x4FC */ u32 maxLength;
    /* 0x500 */ u8 pad_500[0x504 - 0x500];
    /* 0x504 */ struct EwramData_EntityData *prevLink;
};

/* Lilith's shot: its two afterimage entities (ended together with the shot)
 * and the flight angle (curves by 0x400 per frame during the first frames).
 */
struct LilithShotWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ struct EwramData_EntityData *trail0;
    /* 0x4FC */ struct EwramData_EntityData *trail1;
    /* 0x500 */ s32 angle;
};

/* Bat flight: phase of the vertical bob (Sin / 4 added to y). */
struct BatWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 bobPhase;
};

/* Dryad seed: the attacker that knocked it away; never read. */
struct DryadSeedWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ struct EwramData_EntityData *unusedAttacker;
};

/* Axe Armor's thrown axe (AxeArmorAxe_Update): affine angle and spin step. */
struct AxeArmorAxeWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 angle;
    /* 0x4FC */ u32 spin;
};

/* Bomber Armor bomb (BomberArmorBomb_Update): 16.16 walk speed (from the
 * target distance; halved and reversed on a bounce).
 */
struct BomberArmorBombWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 speed;
};

/* Rock Armor rock (RockArmorRock_Update): 16.16 walk speed (from the target
 * distance; reversed on a bounce).
 */
struct RockArmorRockWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 speed;
};

/* Chronomage cast effect (ChronomageCastEffect_Update): affine scale (grows
 * to 1.0, then shrinks). */
struct ChronomageCastWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 scale;
};

/* Chronomage escape (Chronomage_UpdateEscape): 16.16 walk speed
 * (accelerates to 0x30000).
 */
struct ChronomageWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ u32 escapeSpeed;
};

/* Student Witch: 16.16 walk speed passed to Entity_Walk (eased per step) and
 * the frame counter of the current attack step.
 */
struct StudentWitchWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s32 speed;
    /* 0x4FC */ s16 timer;
};

/* Death reward effect: frame counter (its low bit alternates the effect),
 * sprite rotation and affine scale (also its orbit radius, grows to 0x10000
 * and shrinks back).
 */
struct DeathRewardWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FA - 0x4F4];
    /* 0x4FA */ s16 counter;
    /* 0x4FC */ s16 rotation;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s32 scale;
};

/* Death debris (spawned when Death is defeated): it spirals into the room
 * point (0x100, 0x80) while shrinking: sprite rotation and orbit angle (both
 * advanced by the growing spin speed), scale, orbit radius, the scale change
 * per frame and the spin speed.
 */
struct DeathDebrisWork {
    /* 0x4F4 */ u8 pad_4F4[0x4FC - 0x4F4];
    /* 0x4FC */ s16 rotation;
    /* 0x4FE */ s16 orbitAngle;
    /* 0x500 */ s32 scale;
    /* 0x504 */ s32 radius;
    /* 0x508 */ s32 scaleStep;
    /* 0x50C */ s16 spin;
};

/* Tiny Devil flight (TinyDevil_UpdateFlight): frames until the next attack,
 * flags (1 / 2 = x / y offset reached, 4 / 8 = the x / y offset lies on the
 * positive / upper side), bob phase of the hovering and a random pixel offset
 * from the target that it flies to (x / y).
 */
struct TinyDevilWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u16 attackTimer;
    /* 0x4FA */ s16 flags;
    /* 0x4FC */ s16 bobPhase;
    /* 0x4FE */ u8 pad_4FE[0x500 - 0x4FE];
    /* 0x500 */ s32 offsetX;
    /* 0x504 */ s32 offsetY;
};

/* Blue Crow: dives made since the approach started; never read. */
struct BlueCrowWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 unusedDiveCount;
};

/* Red Crow (RedCrow_UpdateMovement): counter of wait / flight phases (odd =
 * flying). */
struct RedCrowWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ s8 phase;
};

/* Chaos second form affine effect (Chaos2_CreateAffineEffect,
 * Chaos2AffineEffect_Update; link in ext.chaosChild.link): countdown of its
 * current step (random waits between the pulses).
 */
struct Chaos2EffectWork {
    /* 0x4F4 */ u8 pad_4F4[0x4F8 - 0x4F4];
    /* 0x4F8 */ u8 timer;
};

/* @@ structs @@ */

#endif /* STRUCTS_ENTITY_WORK_H */
