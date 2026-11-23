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

/*
 * Disc (DiscArmorDisc_Update): owner and ring entity, affine angle, orbit
 * angle around the hand, its turn step, orbit radius, a per-state counter
 * (spiral phase while flying out, rolling speed on the floor, bounce speed
 * when dropped), and the x / y movement of the last frame (the y one is kept
 * in the pointer-typed unk_514 field).
 */
#define DISC_ARMOR_DISC_LAST_DX(e) e->unk_510.whole
#define DISC_ARMOR_DISC_LAST_DY(e) e->unk_514.unk_514_32

/*
 * Ring (DiscArmorRing_Update / _Aim / _Draw): angle from the hand to the
 * disc, their distance (sets the ring scale) and the step between the three
 * drawn copies.
 */

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/* Sparks (SpawnSpark in code_080B1678.c, DiscArmorSpark_Update): animation. */

#endif /* DISC_ARMOR_ENTITY_H */
