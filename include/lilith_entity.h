#ifndef LILITH_ENTITY_H
#define LILITH_ENTITY_H

/*
 * Lilith (EnemyLilith*, Lilith_*):
 * - walk frame: frames of the current walk (re-decides after 60);
 * - dodge checked: the dodge chance for the current player attack was already
 *   rolled (cleared when the player stops attacking);
 * - hidden: 1 until it appears (no contact damage while hidden).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define LILITH_WALK_FRAME(e) e->unk_4FC.part8.unk_4FC
#define LILITH_DODGE_CHECKED(e) e->unk_4FC.part8.unk_4FD
#define LILITH_HIDDEN(e) e->unk_4FC.part8.unk_4FF

/*
 * Lunge hit: set by Lilith_DamagePlayerOnContact; a lunge that hit the player
 * holds its animation and chains a second lunge (Lilith_UpdateAttack).
 */
#define LILITH_LUNGE_HIT(e) e->unk_4FC.part8.unk_4FE

#endif /* LILITH_ENTITY_H */
