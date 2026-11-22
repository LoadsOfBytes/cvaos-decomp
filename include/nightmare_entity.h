#ifndef NIGHTMARE_ENTITY_H
#define NIGHTMARE_ENTITY_H

/*
 * Nightmare (running horse):
 * - run speed: 16.16 speed passed to Entity_Walk (accelerates to 0x28000,
 *   brakes by 0x1000);
 * - run timer: frames of the current run (turns after 0x50);
 * - effect index 0 / 1: entity indices of its two effect entities (ended when
 *   it dies);
 * - room x / y: room position saved for Nightmare_Draw.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define NIGHTMARE_RUN_SPEED(e) e->unk_4FC.whole
#define NIGHTMARE_RUN_TIMER(e) e->unk_500.part8.unk_500
#define NIGHTMARE_EFFECT_INDEX_0(e) e->unk_4F8.part8.unk_4F8
#define NIGHTMARE_EFFECT_INDEX_1(e) e->unk_4F8.part8.unk_4F9
#define NIGHTMARE_ROOM_X(e) e->unk_504.part16.unk_504
#define NIGHTMARE_ROOM_Y(e) e->unk_504.part16.unk_506[0]

#endif /* NIGHTMARE_ENTITY_H */
