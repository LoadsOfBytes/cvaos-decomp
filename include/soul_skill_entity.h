#ifndef SOUL_SKILL_ENTITY_H
#define SOUL_SKILL_ENTITY_H

/*
 * Own work slots of single soul attack entities (the common soul slots are in
 * red_soul_entity.h / blue_soul_entity.h). Angles are 16-bit (0x10000 = full
 * turn), scales and speeds 16.16.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/* Ghost spirit / Tiny Devil homing shots: target, speed (accelerating) and heading. */
#define SKILL_GHOST_TARGET(e) e->unk_504.unk_504_entity
#define SKILL_GHOST_SPEED(e) e->unk_508.whole
#define SKILL_GHOST_HEADING(e) e->unk_510.part16.unk_510
#define SKILL_TINY_DEVIL_TARGET(e) e->unk_504.unk_504_entity
#define SKILL_TINY_DEVIL_SPEED(e) e->unk_508.whole
#define SKILL_TINY_DEVIL_HEADING(e) e->unk_510.part16.unk_510

/*
 * Death's sickle (SkillDeathSickleUpdate): flight speed and heading, spinning
 * affine angle / spin speed, scale (grows to 1.0) and the homing target.
 */
#define SKILL_DEATH_SICKLE_SPEED(e) e->unk_500.whole
#define SKILL_DEATH_SICKLE_SCALE(e) e->unk_50C.whole
#define SKILL_DEATH_SICKLE_SPIN_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_DEATH_SICKLE_SPIN_SPEED(e) e->unk_510.part16.unk_512
#define SKILL_DEATH_SICKLE_TARGET(e) e->unk_514.unk_514_32
#define SKILL_DEATH_SICKLE_HEADING(e) e->unk_518.part16.hp

/*
 * Alastor sword familiar (SkillAlastorUse): affine angle, spin step, scale,
 * attack target and the random y offset aimed at on it.
 */
#define SKILL_ALASTOR_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_ALASTOR_SPIN(e) e->unk_510.part16.unk_512
#define SKILL_ALASTOR_SCALE(e) e->unk_50C.whole
#define SKILL_ALASTOR_TARGET(e) e->unk_514.unk_514_32
#define SKILL_ALASTOR_AIM_Y(e) e->unk_518.whole

/*
 * Red Minotaur axe swing (SkillRedMinotaurUse, read by its hit box): affine
 * angle, swing speed and swing acceleration, scale.
 */
#define SKILL_RED_MINOTAUR_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_RED_MINOTAUR_SWING_SPEED(e) e->unk_510.part16.unk_512
#define SKILL_RED_MINOTAUR_SWING_ACCEL(e) e->unk_50C.whole
#define SKILL_RED_MINOTAUR_SCALE(e) e->unk_508.whole

/*
 * Winged Skeleton spear (SkillWingedSkeletonUse): affine angle, turn step and
 * the spear tip offset that its hit box follows.
 */
#define SKILL_WINGED_SKELETON_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_WINGED_SKELETON_TURN_STEP(e) e->unk_510.part16.unk_512
#define SKILL_WINGED_SKELETON_TIP_X(e) e->unk_508.whole
#define SKILL_WINGED_SKELETON_TIP_Y(e) e->unk_50C.whole

/* Gladiator spinning blade (SkillGladiatorUse): walk speed and affine angle. */
#define SKILL_GLADIATOR_SPEED(e) e->unk_508.whole
#define SKILL_GLADIATOR_ANGLE(e) e->unk_510.part16.unk_510

/*
 * Chronomage clock (SkillChronomageUse): affine scale, palette inversion level
 * and a second ramp that is updated like the scale but never used.
 */
#define SKILL_CHRONOMAGE_SCALE(e) e->unk_504.whole
#define SKILL_CHRONOMAGE_RAMP(e) e->unk_508.whole
#define SKILL_CHRONOMAGE_INVERT(e) e->unk_50C.whole

/*
 * Thrown objects of the Skeleton-family red souls (bone, grenade, rock...,
 * Skill_Skeleton_..._Use, BoneSoulInfo table): 16.16 throw speed (used as
 * the walk speed of rolling objects), affine angle and spin.
 */
#define SKILL_THROW_SPEED(e) e->unk_508.whole
#define SKILL_THROW_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_THROW_SPIN(e) e->unk_510.part16.unk_512

/* Witch / Giant Ghost soul (Skill_Witch_GiantGhost_Use): affine scale. */
#define SKILL_WITCH_SCALE(e) e->unk_50C.whole

/*
 * Boomerang / Axe Armor axe (SkillBoomerangProjectileUpdate, SkillAxeArmorUse):
 * spin direction (+1 / -1) and the spin counter (affine angle << 7 / << 11).
 * Ukoback fireball soul (SkillUkobackUse): affine scale.
 */
#define SKILL_BOOMERANG_SPIN_DIR(e) e->unk_508.part16.unk_508
#define SKILL_BOOMERANG_SPIN(e) e->unk_50C.part16.unk_50E
#define SKILL_UKOBACK_SCALE(e) e->unk_508.whole

/*
 * Disc Armor soul (SkillDiscArmorUse, its scaled sprite reads the parent's
 * angle): angle, turn step, orbit radius and affine scale (pulsing).
 * Cockatrice soul (SkillCockatriceUse / Draw): beam angle, turn step and y
 * scale. Blue Crow soul (SkillBlueCrowUse): angle, turn step, orbit radius.
 */
#define SKILL_DISC_ARMOR_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_DISC_ARMOR_TURN(e) e->unk_510.part16.unk_512
#define SKILL_DISC_ARMOR_RADIUS(e) e->unk_508.whole
#define SKILL_DISC_ARMOR_SCALE(e) e->unk_50C.whole
#define SKILL_COCKATRICE_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_COCKATRICE_TURN(e) e->unk_510.part16.unk_512
#define SKILL_COCKATRICE_SCALE_Y(e) e->unk_508.whole
#define SKILL_BLUE_CROW_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_BLUE_CROW_TURN(e) e->unk_510.part16.unk_512
#define SKILL_BLUE_CROW_RADIUS(e) e->unk_508.whole

/*
 * Slime soul (SkillSlimeUse, SkillSlimeMoveAndBounce): bounce angle (also read
 * as the whole word), 16.16 speed and the lifetime counter.
 */
#define SKILL_SLIME_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_SLIME_ANGLE_WORD(e) e->unk_510.whole
#define SKILL_SLIME_SPEED(e) e->unk_518.whole
#define SKILL_SLIME_TIMER(e) e->unk_508.part16.unk_508

/*
 * Homing soul projectile (HomingSoulProjectile_Update): target, heading and
 * 16.16 speed (accelerates to 0x60000).
 */
#define HOMING_SOUL_TARGET(e) e->unk_504.unk_504_entity
#define HOMING_SOUL_HEADING(e) e->unk_510.whole
#define HOMING_SOUL_SPEED(e) e->unk_518.whole

/*
 * Creaking Skull soul (SkillCreakingSkullUse): pointer to the current angle
 * list (u16 per segment), frame list pointer, scale, active flag and frame
 * index. Its segments (SkillCreakingSkullSegment / HitBox_Update): entity index
 * of the soul, segment index, angle and per-frame turn step.
 */
#define SKILL_CREAKING_SKULL_ANGLES(e) e->unk_510.whole
#define SKILL_CREAKING_SKULL_FRAMES(e) e->unk_518.whole
#define SKILL_CREAKING_SKULL_SCALE(e) e->unk_50C.whole
#define SKILL_CREAKING_SKULL_ACTIVE(e) e->unk_514.part8.unk_514
#define SKILL_CREAKING_SKULL_FRAME(e) e->unk_514.part8.unk_515
#define SKILL_CREAKING_SKULL_SOUL_INDEX(e) e->unk_514.part8.unk_514
#define SKILL_CREAKING_SKULL_SEGMENT(e) e->unk_514.part8.unk_515
#define SKILL_CREAKING_SKULL_SEGMENT_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_CREAKING_SKULL_SEGMENT_TURN(e) e->unk_510.part16.unk_512

/* Bone Pillar soul (SkillBonePillarUse): index 0..2 of the next launched bone. */
#define SKILL_BONE_PILLAR_SHOT(e) e->unk_508.part16.unk_50A

/*
 * Small red souls: Ripper spin (affine angle in 1/20 turns), Nemesis lifetime
 * (300 frames), Siren sway phase (Sin of it is the vertical speed), Une /
 * Biphron walk speed (Une bounces back with half of it), Nightmare and Kyoma
 * Demon horizontal affine scale, Kyoma Demon contact-immunity wait (180
 * frames), Alura Une / Shadow Knight / Big Golem affine x / y scale growing to
 * 1.0, Catoblepas shot index (0..2, launch speed table).
 */
#define SKILL_RIPPER_SPIN(e) e->unk_508.part16.unk_508
#define SKILL_NEMESIS_TIMER(e) e->unk_508.part16.unk_508
#define SKILL_SIREN_PHASE(e) e->unk_508.part16.unk_508
#define SKILL_UNE_SPEED(e) e->unk_508.whole
#define SKILL_BIPHRON_SPEED(e) e->unk_508.whole
#define SKILL_NIGHTMARE_SCALE_X(e) e->unk_508.whole
#define SKILL_KYOMA_DEMON_SCALE_X(e) e->unk_508.whole
#define SKILL_KYOMA_DEMON_TIMER(e) e->unk_508.part16.unk_508
#define SKILL_ALURA_UNE_SCALE_X(e) e->unk_50C.whole
#define SKILL_ALURA_UNE_SCALE_Y(e) e->unk_510.whole
#define SKILL_SHADOW_KNIGHT_SCALE_X(e) e->unk_50C.whole
#define SKILL_SHADOW_KNIGHT_SCALE_Y(e) e->unk_510.whole
#define SKILL_BIG_GOLEM_SCALE_X(e) e->unk_50C.whole
#define SKILL_BIG_GOLEM_SCALE_Y(e) e->unk_510.whole
#define SKILL_CATOBLEPAS_SHOT_INDEX(e) e->unk_508.part16.unk_50A

/*
 * Flame Demon fireball: affine angle and spin step (sign from the facing).
 * Buer orbiting skulls (SkillBuerUse / SkillBuerOrbitUpdate): orbit angle and
 * speed around the soul. Creaking Skull: swing angle of the head the segments
 * and the hit box hang from. Imp: the target it flies to and the y offset
 * above it.
 */
#define SKILL_FLAME_DEMON_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_FLAME_DEMON_SPIN(e) e->unk_510.part16.unk_512
#define SKILL_BUER_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_BUER_SPEED(e) e->unk_510.part16.unk_512
#define SKILL_CREAKING_SKULL_ANGLE(e) e->unk_510.part16.unk_510
#define SKILL_IMP_TARGET(e) e->unk_514.unk_514_32
#define SKILL_IMP_AIM_Y(e) e->unk_518.whole

/* Bone souls (Skill_Skeleton_..._Use): hit flag of the bone projectile, set
 * by BoneSoul_OnPlayerContact; with info flag 0x100 a hit bone shatters. */
#define SKILL_BONE_HIT(e) e->unk_504.part16.unk_504

#endif /* SOUL_SKILL_ENTITY_H */
