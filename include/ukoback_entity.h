#ifndef UKOBACK_ENTITY_H
#define UKOBACK_ENTITY_H

/*
 * Ukoback (EnemyUkobackCreate, Ukoback_UpdateFlight): frames until the next
 * fireball (random 80..439) and the number of its live fireballs (at most 8;
 * a fireball decrements it when it goes out).
 */
#define UKOBACK_ATTACK_TIMER(e) e->unk_4F8.part16.unk_4FA
#define UKOBACK_FIREBALL_COUNT(e) e->unk_504.part8.unk_506

/*
 * Fireball (UkobackFireball_*): affine x / y scale, frame counter and the
 * Ukoback's entity index (its index halfword copied).
 */
#define UKOBACK_FIREBALL_SCALE_X(e) e->unk_4F8.whole
#define UKOBACK_FIREBALL_SCALE_Y(e) e->unk_4FC.whole
#define UKOBACK_FIREBALL_FRAME(e) e->unk_500.part16.unk_500
#define UKOBACK_FIREBALL_OWNER_INDEX(e) e->unk_4F4.part16.unk_4F6

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/* Contact sparkles (Ukoback_SpawnContactSparkle, UkobackSparkle_Update): 16.16 offset from the player. */
#define UKOBACK_SPARKLE_OFFSET_X(e) e->unk_4F8.whole
#define UKOBACK_SPARKLE_OFFSET_Y(e) e->unk_4FC.whole

/* Death pieces (Ukoback_SpawnDeathPiece, UkobackDeathPiece_Update): the
 * Ukoback's entity index (its index halfword copied). */
#define UKOBACK_PIECE_OWNER_INDEX(e) e->unk_4F4.part16.unk_4F6

#endif /* UKOBACK_ENTITY_H */
