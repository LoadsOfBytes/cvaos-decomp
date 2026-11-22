#ifndef DISC_ARMOR_ENTITY_H
#define DISC_ARMOR_ENTITY_H

/*
 * Disc Armor (EnemyDiscArmorUpdate, DiscArmor_UpdateThrow):
 * - flags: bit 0 = disc thrown, bit 1 = the disc is back (set by the disc);
 * - hand offset x / y: pixel offset of the throwing hand (-20 / -36 while
 *   throwing, eased back to -10 / -16);
 * - hand x / y: 16.16 position of the hand, recomputed every frame; the disc
 *   flies around it and the ring aims from it.
 */
#define DISC_ARMOR_FLAGS(e) e->unk_4F8.whole
#define DISC_ARMOR_HAND_OFFSET_X(e) e->unk_504.part16.unk_504
#define DISC_ARMOR_HAND_OFFSET_Y(e) e->unk_504.part16.unk_506[0]
#define DISC_ARMOR_HAND_X(e) e->unk_4FC.whole
#define DISC_ARMOR_HAND_Y(e) e->unk_500.whole

/*
 * Disc (DiscArmorDisc_Update): owner and ring entity, affine angle, orbit
 * angle around the hand, its turn step, orbit radius, a per-state counter
 * (spiral phase while flying out, rolling speed on the floor, bounce speed
 * when dropped), and the x / y movement of the last frame (the y one is kept
 * in the pointer-typed unk_514 field).
 */
#define DISC_ARMOR_DISC_OWNER(e) e->unk_4F4.unk_4F4_entity
#define DISC_ARMOR_DISC_RING(e) e->unk_4FC.unk_4FC_entity
#define DISC_ARMOR_DISC_ANGLE(e) e->unk_4F8.whole
#define DISC_ARMOR_DISC_ORBIT_ANGLE(e) e->unk_500.whole
#define DISC_ARMOR_DISC_TURN(e) e->unk_504.whole
#define DISC_ARMOR_DISC_RADIUS(e) e->unk_508.whole
#define DISC_ARMOR_DISC_PHASE(e) e->unk_50C.whole
#define DISC_ARMOR_DISC_LAST_DX(e) e->unk_510.whole
#define DISC_ARMOR_DISC_LAST_DY(e) e->unk_514.unk_514_32

/*
 * Ring (DiscArmorRing_Update / _Aim / _Draw): angle from the hand to the
 * disc, their distance (sets the ring scale) and the step between the three
 * drawn copies.
 */
#define DISC_ARMOR_RING_ANGLE(e) e->unk_4F8.whole
#define DISC_ARMOR_RING_LENGTH(e) e->unk_504.whole
#define DISC_ARMOR_RING_STEP_X(e) e->unk_4FC.whole
#define DISC_ARMOR_RING_STEP_Y(e) e->unk_500.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* DISC_ARMOR_ENTITY_H */
