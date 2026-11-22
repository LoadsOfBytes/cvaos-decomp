#ifndef GOLEM_ENTITY_H
#define GOLEM_ENTITY_H

/*
 * Golem family (EnemyGolemCreate, Golem_*; joint slots in joint_entity.h):
 * - variant: index into the per-variant timing tables (from the enemy id,
 *   0..3);
 * - attack phase: 0 during the swing, 1 when the punch lands (read by the
 *   golem segments' contact handler).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GOLEM_VARIANT(e) e->unk_508.whole
#define GOLEM_ATTACK_PHASE(e) e->unk_4FC.part8.unk_4FC

/*
 * Body segments (GolemSegment_Update; part index / anchor / owner in
 * joint_entity.h): spin speed and affine angle of the fall after death.
 */
#define GOLEM_SEGMENT_SPIN(e) e->unk_4F8.whole
#define GOLEM_SEGMENT_FALL_ANGLE(e) e->unk_4FC.whole

#endif /* GOLEM_ENTITY_H */
