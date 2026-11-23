#ifndef SOUL_SKILL_ENTITY_H
#define SOUL_SKILL_ENTITY_H

/*
 * Own fields of single soul attack entities. Their work-area slots are in
 * ext.skill<Soul> (include/structs/entity_work.h; the common soul slots in
 * ext.redSoul / ext.blueSoul); the macros below name the entity fields after
 * the work area (common block 0x510..0x51B: ENTITY_HALF_510, mp, spawnParam0, hp ...). Angles are 16-bit (0x10000 = full turn),
 * scales and speeds 16.16.
 * The macros expand to member chains or to the ENTITY_* views of the common
 * block (structs/ewram.h) without added parentheses; use them with a plain
 * base expression.
 */

/* Ghost spirit / Tiny Devil homing shots: target, speed (accelerating) and heading. */
#define SKILL_GHOST_HEADING(e) ENTITY_HALF_510(e)
#define SKILL_TINY_DEVIL_HEADING(e) ENTITY_HALF_510(e)

/*
 * Death's sickle (SkillDeathSickleUpdate): flight speed and heading, spinning
 * affine angle / spin speed, scale (grows to 1.0) and the homing target.
 */
#define SKILL_DEATH_SICKLE_SPIN_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_DEATH_SICKLE_SPIN_SPEED(e) e->mp
#define SKILL_DEATH_SICKLE_TARGET(e) ENTITY_PTR_514(e)
#define SKILL_DEATH_SICKLE_HEADING(e) e->hp

/*
 * Alastor sword familiar (SkillAlastorUse): affine angle, spin step, scale,
 * attack target and the random y offset aimed at on it.
 */
#define SKILL_ALASTOR_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_ALASTOR_SPIN(e) e->mp
#define SKILL_ALASTOR_TARGET(e) ENTITY_PTR_514(e)
#define SKILL_ALASTOR_AIM_Y(e) ENTITY_WORD_518(e)

/*
 * Red Minotaur axe swing (SkillRedMinotaurUse, read by its hit box): affine
 * angle, swing speed and swing acceleration, scale.
 */
#define SKILL_RED_MINOTAUR_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_RED_MINOTAUR_SWING_SPEED(e) e->mp

/*
 * Winged Skeleton spear (SkillWingedSkeletonUse): affine angle, turn step and
 * the spear tip offset that its hit box follows.
 */
#define SKILL_WINGED_SKELETON_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_WINGED_SKELETON_TURN_STEP(e) e->mp

/* Gladiator spinning blade (SkillGladiatorUse): walk speed and affine angle. */
#define SKILL_GLADIATOR_ANGLE(e) ENTITY_HALF_510(e)

/*
 * Chronomage clock (SkillChronomageUse): affine scale, palette inversion level
 * and a second ramp that is updated like the scale but never used.
 */

/*
 * Thrown objects of the Skeleton-family red souls (bone, grenade, rock...,
 * Skill_Skeleton_..._Use, BoneSoulInfo table): 16.16 throw speed (used as
 * the walk speed of rolling objects), affine angle and spin.
 */
#define SKILL_THROW_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_THROW_SPIN(e) e->mp

/* Witch / Giant Ghost soul (Skill_Witch_GiantGhost_Use): affine scale. */

/*
 * Boomerang / Axe Armor axe (SkillBoomerangProjectileUpdate, SkillAxeArmorUse):
 * spin direction (+1 / -1) and the spin counter (affine angle << 7 / << 11).
 * Ukoback fireball soul (SkillUkobackUse): affine scale.
 */

/*
 * Disc Armor soul (SkillDiscArmorUse, its scaled sprite reads the parent's
 * angle): angle, turn step, orbit radius and affine scale (pulsing).
 * Cockatrice soul (SkillCockatriceUse / Draw): beam angle, turn step and y
 * scale. Blue Crow soul (SkillBlueCrowUse): angle, turn step, orbit radius.
 */
#define SKILL_DISC_ARMOR_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_DISC_ARMOR_TURN(e) e->mp
#define SKILL_COCKATRICE_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_COCKATRICE_TURN(e) e->mp
#define SKILL_BLUE_CROW_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_BLUE_CROW_TURN(e) e->mp

/*
 * Slime soul (SkillSlimeUse, SkillSlimeMoveAndBounce): bounce angle (also read
 * as the whole word), 16.16 speed and the lifetime counter.
 */
#define SKILL_SLIME_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_SLIME_ANGLE_WORD(e) ENTITY_WORD_510(e)
#define SKILL_SLIME_SPEED(e) ENTITY_WORD_518(e)

/*
 * Homing soul projectile (HomingSoulProjectile_Update): target, heading and
 * 16.16 speed (accelerates to 0x60000).
 */
#define HOMING_SOUL_HEADING(e) ENTITY_WORD_510(e)
#define HOMING_SOUL_SPEED(e) ENTITY_WORD_518(e)

/*
 * Creaking Skull soul (SkillCreakingSkullUse): pointer to the current angle
 * list (u16 per segment), frame list pointer, scale, active flag and frame
 * index. Its segments (SkillCreakingSkullSegment / HitBox_Update): entity index
 * of the soul, segment index, angle and per-frame turn step.
 */
#define SKILL_CREAKING_SKULL_ANGLES(e) ENTITY_WORD_510(e)
#define SKILL_CREAKING_SKULL_FRAMES(e) ENTITY_WORD_518(e)
#define SKILL_CREAKING_SKULL_ACTIVE(e) ENTITY_BYTE_514(e)
#define SKILL_CREAKING_SKULL_FRAME(e) ENTITY_BYTE_515(e)
#define SKILL_CREAKING_SKULL_SOUL_INDEX(e) ENTITY_BYTE_514(e)
#define SKILL_CREAKING_SKULL_SEGMENT(e) ENTITY_BYTE_515(e)
#define SKILL_CREAKING_SKULL_SEGMENT_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_CREAKING_SKULL_SEGMENT_TURN(e) e->mp

/* Bone Pillar soul (SkillBonePillarUse): index 0..2 of the next launched bone. */

/*
 * Small red souls: Ripper spin (affine angle in 1/20 turns), Nemesis lifetime
 * (300 frames), Siren sway phase (Sin of it is the vertical speed), Une /
 * Biphron walk speed (Une bounces back with half of it), Nightmare and Kyoma
 * Demon horizontal affine scale, Kyoma Demon contact-immunity wait (180
 * frames), Alura Une / Shadow Knight / Big Golem affine x / y scale growing to
 * 1.0, Catoblepas shot index (0..2, launch speed table).
 */
#define SKILL_ALURA_UNE_SCALE_Y(e) ENTITY_WORD_510(e)
#define SKILL_SHADOW_KNIGHT_SCALE_Y(e) ENTITY_WORD_510(e)
#define SKILL_BIG_GOLEM_SCALE_Y(e) ENTITY_WORD_510(e)

/*
 * Flame Demon fireball: affine angle and spin step (sign from the facing).
 * Buer orbiting skulls (SkillBuerUse / SkillBuerOrbitUpdate): orbit angle and
 * speed around the soul. Creaking Skull: swing angle of the head the segments
 * and the hit box hang from. Imp: the target it flies to and the y offset
 * above it.
 */
#define SKILL_FLAME_DEMON_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_FLAME_DEMON_SPIN(e) e->mp
#define SKILL_BUER_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_BUER_SPEED(e) e->mp
#define SKILL_CREAKING_SKULL_ANGLE(e) ENTITY_HALF_510(e)
#define SKILL_IMP_TARGET(e) ENTITY_PTR_514(e)
#define SKILL_IMP_AIM_Y(e) ENTITY_WORD_518(e)

/* Bone souls (Skill_Skeleton_..._Use): hit flag of the bone projectile, set
 * by BoneSoul_OnPlayerContact; with info flag 0x100 a hit bone shatters. */

#endif /* SOUL_SKILL_ENTITY_H */
