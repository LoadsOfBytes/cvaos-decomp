#ifndef FLEAMAN_ENTITY_H
#define FLEAMAN_ENTITY_H

/*
 * Fleaman (EnemyFleaman*, Fleaman_*): airborne (1 when both spawn params are
 * 0xFF: starts in the air instead of snapping to the ground) and the index
 * of the animation it last started (0 / 2 / 3).
 */
#define FLEAMAN_AIRBORNE(e) e->unk_4F8.part8.unk_4F8
#define FLEAMAN_ANIM(e) e->unk_4F8.part8.unk_4F9

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* FLEAMAN_ENTITY_H */
