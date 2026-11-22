#ifndef DEAD_WARRIOR_ENTITY_H
#define DEAD_WARRIOR_ENTITY_H

/*
 * Dead Warrior (DeadWarrior_*): 16.16 charge walk speed (accelerates to
 * 0x20000, brakes by 0x1000) and the frame counter of the charge (stops after
 * 100; Create starts it at 50).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define DEAD_WARRIOR_SPEED(e) e->unk_4FC.whole
#define DEAD_WARRIOR_CHARGE_FRAME(e) e->unk_500.part8.unk_500

#endif /* DEAD_WARRIOR_ENTITY_H */
