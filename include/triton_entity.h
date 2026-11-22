#ifndef TRITON_ENTITY_H
#define TRITON_ENTITY_H

/*
 * Triton (Triton_UpdateMove / _UpdateAttack - TU aliases NEXT_STATE /
 * SPEAR_FLAG): its spear, the sub-state the move code continues with, and the
 * thrust flag - set on the Triton to raise the spear, set on the spear by
 * TritonSpear_Update when the thrust is done (each side clears the other).
 * Spear (Triton_CreateSpear, TritonSpear_Update - TU alias PART_ANGLE): the
 * Triton holding it and its affine angle. Homing shot: the Triton that fired
 * it. Wave particles (TU aliases WAVE_ANGLE / WAVE_AGE): sway angle and age.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define TRITON_SPEAR(e) e->unk_500.unk_500_entity
#define TRITON_NEXT_STATE(e) e->unk_4FC.part8.unk_4FD
#define TRITON_THRUST(e) e->unk_4FC.part8.unk_4FC
#define TRITON_SPEAR_OWNER(e) e->unk_500.unk_500_entity
#define TRITON_SPEAR_ANGLE(e) e->unk_504.whole
#define TRITON_SHOT_OWNER(e) e->unk_500.unk_500_entity
#define TRITON_WAVE_ANGLE(e) e->unk_4F8.whole
#define TRITON_WAVE_AGE(e) e->unk_500.whole

#endif /* TRITON_ENTITY_H */
