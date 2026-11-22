#ifndef GHOST_DANCER_ENTITY_H
#define GHOST_DANCER_ENTITY_H

/*
 * Ghost Dancer (EnemyGhostDancerCreate, GhostDancer_*; the TU aliases PHASE /
 * MODE / TIMER read the same bytes): sine phase of the dance (+128 per frame),
 * number of completed cycles, the side it sways to (toggles), dance mode
 * (advanced every half cycle) and the mode timer.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GHOST_DANCER_PHASE(e) e->unk_4F8.part16.unk_4F8
#define GHOST_DANCER_CYCLES(e) e->unk_4F8.part8.unk_4FA
#define GHOST_DANCER_SIDE(e) e->unk_4F8.part8.unk_4FB
#define GHOST_DANCER_MODE(e) e->unk_4FC.part8.unk_4FC
#define GHOST_DANCER_TIMER(e) e->unk_4FC.part8.unk_4FD

#endif /* GHOST_DANCER_ENTITY_H */
