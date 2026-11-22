#ifndef KILLER_MANTLE_ENTITY_H
#define KILLER_MANTLE_ENTITY_H

/*
 * Killer Mantle (KillerMantle_*):
 * - speed: 16.16 walk speed (hover sway amplitude, random per sway; jump
 *   attack speed, braked to 0);
 * - sway frame: frame of the current hover sway (turns after 48);
 * - flags: bit 0 set during the jump attack (spreads the cape part).
 */
#define KILLER_MANTLE_SPEED(e) e->unk_4F8.whole
#define KILLER_MANTLE_SWAY_FRAME(e) e->unk_4FC.part16.unk_4FC
#define KILLER_MANTLE_FLAGS(e) e->unk_4FC.part16.unk_4FE[0]

/* Cape part (KillerMantlePart_Update): affine angle and x scale. */
#define KILLER_MANTLE_PART_ANGLE(e) e->unk_4FC.part16.unk_4FE[0]
#define KILLER_MANTLE_PART_SCALE_X(e) e->unk_500.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* KILLER_MANTLE_ENTITY_H */
