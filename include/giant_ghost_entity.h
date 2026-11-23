#ifndef GIANT_GHOST_ENTITY_H
#define GIANT_GHOST_ENTITY_H

/*
 * Giant Ghost body (GiantGhost_*): 16.16 speed of the back-off swoop, the
 * room position of the player stored for the dive, and the bit-player flag
 * (set by the contact handler, cleared when the attack ends).
 */
#define GIANT_GHOST_SPEED(e) e->unk_4F8.whole
#define GIANT_GHOST_DIVE_X(e) e->unk_4FC.whole
#define GIANT_GHOST_DIVE_Y(e) e->unk_500.whole
#define GIANT_GHOST_HIT_PLAYER(e) e->unk_504.part8.unk_505
/* Cleared by GiantGhost_ResetAnimation, never read for the body. */
#define GIANT_GHOST_UNUSED_FLAG(e) e->unk_504.part8.unk_504

/*
 * Hands (GiantGhostHandA/B_Update, created in EnemyGiantGhostCreate): hand
 * index, the room point they turn towards (swaying by one pixel per frame),
 * sway direction and sway frame (flips after 60 frames).
 */
#define GIANT_GHOST_HAND_INDEX(e) e->unk_504.part8.unk_504
#define GIANT_GHOST_HAND_POINT_X(e) e->unk_4FC.whole
#define GIANT_GHOST_HAND_POINT_Y(e) e->unk_500.whole
#define GIANT_GHOST_HAND_SWAY_DIR(e) e->unk_504.part8.unk_506
#define GIANT_GHOST_HAND_SWAY_FRAME(e) e->unk_504.part8.unk_507

/* Aura (GiantGhostAura_Update): pulse phase. */
#define GIANT_GHOST_AURA_PHASE(e) e->unk_500.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* GIANT_GHOST_ENTITY_H */
