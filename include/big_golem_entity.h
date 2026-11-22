#ifndef BIG_GOLEM_ENTITY_H
#define BIG_GOLEM_ENTITY_H

/*
 * Big Golem parts (BigGolemPart_Update; joint slots in joint_entity.h): the
 * owner (whose unk_500.bigGolemParts holds the joint array) and, once the
 * golem is dead, the falling piece's spin, affine angle and floor y.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BIG_GOLEM_PART_OWNER(e) e->unk_500.unk_500_entity
#define BIG_GOLEM_PART_SPIN(e) e->unk_4F4.whole
#define BIG_GOLEM_PART_ANGLE(e) e->unk_4FC.whole
#define BIG_GOLEM_PART_FLOOR_Y(e) e->unk_508.whole

/*
 * Body: hit / hit count (set and incremented, up to 10, by BigGolem_OnDamaged)
 * and its two palette slots.
 */
#define BIG_GOLEM_HIT(e) e->unk_4FC.part8.unk_4FE
#define BIG_GOLEM_HIT_COUNT(e) e->unk_4FC.part8.unk_4FD
#define BIG_GOLEM_PALETTE_0(e) e->unk_508.part8.unk_50A
#define BIG_GOLEM_PALETTE_1(e) e->unk_508.part8.unk_50B

/*
 * Walk (GreatArmor-like TU macros WALL_CHECK / RANDOM_TURN of
 * BigGolem_UpdateWalk): heading the golem walks in (compared with flipX; set
 * to flipX ^ 1 to turn around at walls, at random near the target or after
 * more than 3 hits).
 */
#define BIG_GOLEM_HEADING(e) e->unk_504.part8.unk_505

/* Attack (BigGolem_UpdateAttack - TU alias VARIANT): punch variant 0 / 1. */
#define BIG_GOLEM_ATTACK_VARIANT(e) e->unk_4F4.part8.unk_4F4

#endif /* BIG_GOLEM_ENTITY_H */
