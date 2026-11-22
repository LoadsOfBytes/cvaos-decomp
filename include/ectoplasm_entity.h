#ifndef ECTOPLASM_ENTITY_H
#define ECTOPLASM_ENTITY_H

/*
 * Ectoplasm (EnemyEctoplasm*, Ectoplasm_*): 16.16 room point it floats to
 * (the spawn point at first, then random points near the target), bob phase
 * of the floating, and the retarget toggle (alternates near / far points).
 */
#define ECTOPLASM_POINT_X(e) e->unk_4F8.whole
#define ECTOPLASM_POINT_Y(e) e->unk_4FC.whole
#define ECTOPLASM_BOB_PHASE(e) e->unk_500.whole
#define ECTOPLASM_RETARGET(e) e->unk_504.part8.unk_504

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* ECTOPLASM_ENTITY_H */
