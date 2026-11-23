#ifndef CHAOS_ENTITY_H
#define CHAOS_ENTITY_H

/*
 * Chaos core entity:
 * - data: address of its heap block (struct ChaosData, struct Chaos2Data in
 *   the second form);
 * - attack timers 0..2: random countdowns after which the core asks an orb to
 *   attack;
 * - active orbs: attack request / orbs currently attacking; busy: an orb
 *   attack is running;
 * - broken orbs: bit per destroyed orb.
 * Chaos orbs keep their orbit angle offset (added to the background rotation).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define CHAOS_DATA(e) e->unk_504.whole
#define CHAOS_ATTACK_TIMER_0(e) e->unk_4FC.part8.unk_4FD
#define CHAOS_ATTACK_TIMER_1(e) e->unk_4FC.part8.unk_4FE
#define CHAOS_ATTACK_TIMER_2(e) e->unk_4FC.part8.unk_4FF
#define CHAOS_ACTIVE_ORBS(e) e->unk_4F8.part8.unk_4F9
#define CHAOS_BUSY(e) e->unk_4F8.part8.unk_4FB
#define CHAOS_BROKEN_ORBS(e) e->unk_4FC.part8.unk_4FC
#define CHAOS_ORB_ANGLE(e) e->unk_4FC.part16.unk_4FC

/*
 * Chaos parts, orbs, shots and effects (both forms) point to the entity that
 * created them (the core, or the part / orb for their own effects).
 */
#define CHAOS_LINK(e) e->unk_4F4.unk_4F4_entity

/*
 * Second-form snake (Chaos2_CreateSnake, Chaos2SnakeHead_Update,
 * Chaos2SnakeSegment_Update): heap block with the head's 120-entry position
 * history (6 bytes per entry), camera x / y low bytes of the last frame
 * (compensates scrolling), segment index (0..7, 7 = tail), affine angle,
 * waypoint target x / y, waypoint set (mode 0..3) and waypoint number, turn
 * speed and the head's move timer.
 */
#define CHAOS_SNAKE_TRAIL(e) e->unk_508.whole
#define CHAOS_SNAKE_CAMERA_X(e) e->unk_4FC.part8.unk_4FC
#define CHAOS_SNAKE_CAMERA_Y(e) e->unk_4FC.part8.unk_4FD
#define CHAOS_SNAKE_INDEX(e) e->unk_4FC.part8.unk_4FE
#define CHAOS_SNAKE_ANGLE(e) e->unk_504.part16.unk_504
#define CHAOS_SNAKE_TARGET_X(e) e->unk_500.part16.unk_500
#define CHAOS_SNAKE_TARGET_Y(e) e->unk_500.part16.unk_502
#define CHAOS_SNAKE_MODE(e) e->unk_4F8.part8.unk_4FB
#define CHAOS_SNAKE_POINT(e) e->unk_4FC.part8.unk_4FF
#define CHAOS_SNAKE_TURN(e) e->unk_4F8.part16.unk_4F8
#define CHAOS_SNAKE_TIMER(e) e->unk_504.part16.unk_506[0]

/*
 * Shots, sparks and seeds of both forms (ChaosSeedShot_*, ChaosSpark_*,
 * ChaosSoulShot_Update, Chaos2Spark_Update ... - TU aliases OFFSET_X /
 * OFFSET_Y): 16.16 offset from the entity they follow (their orb or the enemy
 * target). Seed / soul shots: reflected flag (set when the player hits them
 * back) and the spread of the soul shot.
 */
#define CHAOS_OFFSET_X(e) e->unk_4F8.whole
#define CHAOS_OFFSET_Y(e) e->unk_4FC.whole
#define CHAOS_SHOT_REFLECTED(e) e->unk_504.part8.unk_504
#define CHAOS_SHOT_SPREAD(e) e->unk_504.whole

/*
 * Orbs (Chaos_CreateOrb, ChaosOrb_Update - TU aliases OFS_X / OFS_Y / KIND):
 * offset from the core (pixels), kind (the soul type; its marks, halos and
 * auras keep the kind of their orb - TU aliases KIND / IDX) and the flag the
 * soul shot sets when its animation has ended (the orb reappears).
 */
#define CHAOS_ORB_OFFSET_X(e) e->unk_500.part16.unk_500
#define CHAOS_ORB_OFFSET_Y(e) e->unk_500.part16.unk_502
#define CHAOS_ORB_KIND(e) e->unk_504.part8.unk_506
#define CHAOS_ORB_SHOT_DONE(e) e->unk_504.part8.unk_507

/*
 * Rotating rings (Chaos_CreateOuterRing / InnerRing, *Ring_Update - TU
 * aliases SPIN / ORBIT / SPIN_BASE / ORBIT_BASE / CENTER_X / CENTER_Y /
 * RADIUS): running spin and orbit angles, their fixed offsets per ring, orbit
 * centre (pixels) and radius.
 */
#define CHAOS_RING_SPIN(e) e->unk_4F8.part16.unk_4F8
#define CHAOS_RING_ORBIT(e) e->unk_4F8.part16.unk_4FA
#define CHAOS_RING_SPIN_OFFSET(e) e->unk_4FC.part16.unk_4FC
#define CHAOS_RING_ORBIT_OFFSET(e) e->unk_4FC.part16.unk_4FE[0]
#define CHAOS_RING_CENTER_X(e) e->unk_500.part16.unk_500
#define CHAOS_RING_CENTER_Y(e) e->unk_500.part16.unk_502
#define CHAOS_RING_RADIUS(e) e->unk_504.part16.unk_504

/* Second-form spinning debris (Chaos2SpinningDebris_Update): affine angle,
 * advanced by accelX. */
#define CHAOS_DEBRIS_ANGLE(e) e->unk_4FC.part16.unk_4FC

/*
 * Chaos second form core: live parts (4 at first, lowered by Chaos2Part_OnHit)
 * and its base defense (Chaos2_OnHit uses parts * 20 + base); defeat flag set
 * by its affine effect when it has risen (written there as bytes[1]), the
 * defeat sequence (Chaos2_UpdateDefeat) waits for it. Parts: hit flash
 * frames (30 after a hit; TU alias FLASH).
 * Sparks (ChaosSpark_*): set by ChaosSpark_OnContact when it grabs the player.
 */
#define CHAOS2_PART_COUNT(e) e->unk_500.part8.unk_500
#define CHAOS2_BASE_DEFENSE(e) e->unk_500.part8.unk_501
#define CHAOS2_DEFEAT_READY(e) e->unk_4F8.part8.unk_4F9
#define CHAOS2_PART_FLASH(e) e->unk_504.part16.unk_504
#define CHAOS_SPARK_GRABBED(e) e->unk_500.part8.unk_500

/* CHAOS_DATA read as a pointer (the same word through the entity member):
 * the graphics / palette block of the core, read by its children. */
#define CHAOS_DATA_PTR(e) e->unk_504.unk_504_entity

/* Chaos core: a random 0..63 stored when the fight starts; never read. */
#define CHAOS_UNUSED_RANDOM(e) e->unk_4F8.part8.unk_4FA

#endif /* CHAOS_ENTITY_H */
