#ifndef GLADIATOR_ENTITY_H
#define GLADIATOR_ENTITY_H

/*
 * Gladiator (EnemyGladiator*, Gladiator_*): flinch flag (set when damaged,
 * stops the box animation) and palette slot.
 */
#define GLADIATOR_FLINCH(e) e->unk_4F8.part8.unk_4F8
#define GLADIATOR_PALETTE(e) e->unk_504.part8.unk_504

/*
 * Spinning weapon parts (GladiatorSpinner_Update, created in
 * EnemyGladiatorCreate): part index, pixel offset from the owner, affine
 * angle and spin speed (accelerates up to 0x800).
 */
#define GLADIATOR_SPINNER_INDEX(e) e->unk_4FC.part8.unk_4FC
#define GLADIATOR_SPINNER_OFFSET_X(e) e->unk_4F4.part16.unk_4F4
#define GLADIATOR_SPINNER_OFFSET_Y(e) e->unk_4F4.part16.unk_4F6
#define GLADIATOR_SPINNER_ANGLE(e) e->unk_500.part16.unk_500
#define GLADIATOR_SPINNER_SPIN(e) e->unk_500.part16.unk_502

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/*
 * Body and arm parts (GladiatorBody_Update, GladiatorArm_Update): pixel offset
 * from the owner, vertical bob of the owner's animation step, and the arm's
 * random idle timer. Shot (GladiatorShot_Update): heading, turned towards
 * straight down by 0x100 per frame.
 */
#define GLADIATOR_PART_OFFSET_X(e) e->unk_4F4.part16.unk_4F4
#define GLADIATOR_PART_OFFSET_Y(e) e->unk_4F4.part16.unk_4F6
#define GLADIATOR_PART_BOB_Y(e) e->unk_4FC.part16.unk_4FE[0]
#define GLADIATOR_ARM_TIMER(e) e->unk_500.part16.unk_500
#define GLADIATOR_SHOT_ANGLE(e) e->unk_500.part16.unk_502

/* Second palette variant (variant 1 of sGladiatorPalette, the base palette). */
#define GLADIATOR_PALETTE_1(e) e->unk_504.part8.unk_505

#endif /* GLADIATOR_ENTITY_H */
