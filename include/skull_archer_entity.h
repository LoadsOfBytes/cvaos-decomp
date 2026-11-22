#ifndef SKULL_ARCHER_ENTITY_H
#define SKULL_ARCHER_ENTITY_H

/*
 * Skull Archer (SkullArcher_*): step 0..4 through the shot patterns (index /
 * 2 into sUnk_08528614), the current pattern pointer, and the side (+1 / -1)
 * the killing hit came from (scatters the death pieces).
 * Arrow (SkullArcherArrow_*): the archer that shot it.
 */
#define SKULL_ARCHER_PATTERN_STEP(e) e->unk_4F8.part8.unk_4F8
#define SKULL_ARCHER_PATTERN(e) e->unk_500.whole
#define SKULL_ARCHER_HIT_SIDE(e) e->unk_504.whole
#define SKULL_ARCHER_ARROW_OWNER(e) e->unk_4FC.unk_4FC_entity

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* SKULL_ARCHER_ENTITY_H */
