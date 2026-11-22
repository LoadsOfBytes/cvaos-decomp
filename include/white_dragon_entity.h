#ifndef WHITE_DRAGON_ENTITY_H
#define WHITE_DRAGON_ENTITY_H

/*
 * Work slots of the White Dragon head (the body segments are drawn from it):
 * - history: address of a heap buffer with the recent head angles; segment n
 *   uses an older entry, which makes the neck follow the head;
 * - anchor x / y: room position of the spawn point (camera position when it
 *   was created); offset x / y: head position relative to the anchor;
 * - sway: amplitude of the neck wave (eased towards 128 / 256); sway phase:
 *   phase of the side-to-side sway.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define WHITE_DRAGON_HISTORY(e) e->unk_504.whole
#define WHITE_DRAGON_ANCHOR_X(e) e->unk_4FC.part16.unk_4FC
#define WHITE_DRAGON_ANCHOR_Y(e) e->unk_4FC.part16.unk_4FE[0]
#define WHITE_DRAGON_OFFSET_X(e) e->unk_4F8.part16.unk_4F8
#define WHITE_DRAGON_OFFSET_Y(e) e->unk_4F8.part16.unk_4FA
#define WHITE_DRAGON_SWAY(e) e->unk_500.part16.unk_500
#define WHITE_DRAGON_SWAY_PHASE(e) e->unk_500.part16.unk_502

/*
 * Idle timers / aim point (same slots): while idle, move timer and shot timer
 * count down to the next move / shot decision; when an attack starts they are
 * overwritten with the room pixel point it aims at (target, 20 px up), and the
 * attack restarts the timers when it ends.
 */
#define WHITE_DRAGON_MOVE_TIMER(e) e->unk_4F4.part16.unk_4F4
#define WHITE_DRAGON_SHOT_TIMER(e) e->unk_4F4.part16.unk_4F6
#define WHITE_DRAGON_AIM_X(e) e->unk_4F4.part16.unk_4F4
#define WHITE_DRAGON_AIM_Y(e) e->unk_4F4.part16.unk_4F6

/*
 * Body segments (EnemyWhiteDragonCreate, WhiteDragonSegment_* - TU aliases
 * SEGMENT): the previous segment towards the head and the segment index
 * (selects the history entry and the distance from the head).
 */
#define WHITE_DRAGON_SEGMENT_PREV(e) e->unk_4FC.unk_4FC_entity
#define WHITE_DRAGON_SEGMENT_INDEX(e) e->unk_500.part8.unk_500

/*
 * Charge particles (WhiteDragon_SpawnChargeParticle,
 * WhiteDragonChargeParticle_Update): room pixel point they converge on.
 */
#define WHITE_DRAGON_CHARGE_TARGET_X(e) e->unk_4F8.part16.unk_4F8
#define WHITE_DRAGON_CHARGE_TARGET_Y(e) e->unk_4F8.part16.unk_4FA

#endif /* WHITE_DRAGON_ENTITY_H */
