#ifndef CREAKING_SKULL_ENTITY_H
#define CREAKING_SKULL_ENTITY_H

/*
 * Creaking Skull (EnemyCreakingSkull*, CreakingSkull_*; joint slots in
 * joint_entity.h):
 * - variant: index into the per-variant timing tables (0 = enemy 0x21,
 *   1 = enemy 0x5C);
 * - pose delay: frames until the mid-step pose switch (half the step timer).
 */
#define CREAKING_SKULL_VARIANT(e) e->unk_508.whole
#define CREAKING_SKULL_POSE_DELAY(e) e->unk_4FC.part8.unk_4FD

/*
 * Bone part after the skull dies (CreakingSkullPart_Update): spin speed,
 * floor y it bounces on, and its affine angle.
 */
#define CREAKING_SKULL_PART_SPIN(e) e->unk_4F4.whole
#define CREAKING_SKULL_PART_FLOOR_Y(e) e->unk_4F8.whole
#define CREAKING_SKULL_PART_ANGLE(e) e->unk_4FC.whole

/*
 * Guided effect (CreakingSkullGuidedEffect_Update): room positions it heads
 * for, first the early target, in its last 7 frames the late target.
 */
#define CREAKING_SKULL_GUIDE_LATE_X(e) e->unk_4F8.whole
#define CREAKING_SKULL_GUIDE_LATE_Y(e) e->unk_4FC.whole
#define CREAKING_SKULL_GUIDE_EARLY_X(e) e->unk_500.whole
#define CREAKING_SKULL_GUIDE_EARLY_Y(e) e->unk_504.whole

/* Bone projectile (CreakingSkullProjectile_Update): angle of its orbiting particles. */
#define CREAKING_SKULL_PROJECTILE_ORBIT(e) e->unk_4FC.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* CREAKING_SKULL_ENTITY_H */
