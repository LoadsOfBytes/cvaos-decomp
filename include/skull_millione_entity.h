#ifndef SKULL_MILLIONE_ENTITY_H
#define SKULL_MILLIONE_ENTITY_H

/*
 * Skull Millione family (SkullMillioneFamily_*): variant (index into
 * sSkullMillioneFamilyVariants), bob frame of the wait state and the 16.16
 * approach speed (accelerates to the variant's approachSpeed).
 * Part (SkullMillioneFamilyPart_Update): affine angle.
 */
#define SKULL_MILLIONE_VARIANT(e) e->unk_4F8.part8.unk_4F8
#define SKULL_MILLIONE_BOB_FRAME(e) e->unk_4FC.part16.unk_4FC
#define SKULL_MILLIONE_SPEED(e) e->unk_504.whole
#define SKULL_MILLIONE_PART_ANGLE(e) e->unk_4FC.part16.unk_4FE[0]

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* SKULL_MILLIONE_ENTITY_H */
