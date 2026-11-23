#ifndef ALTAIR_ENTITY_H
#define ALTAIR_ENTITY_H

/*
 * Altair work slots:
 * - carried: the enemy it carries (Altair_CreateCarriedEnemy); carrying: 1
 *   while it still holds it; drop ready: set when it decides to drop it near
 *   the target;
 * - flight phase: 0..3 step of Altair_UpdateFlight (3 = dying);
 * - spawner: the spawner entity that created it; on the spawner, spawn count
 *   is the number of live Altairs (decremented when one is deleted).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define ALTAIR_CARRIED(e) e->unk_4F8.unk_4F8_entity
#define ALTAIR_CARRYING(e) e->unk_500.part8.unk_500
#define ALTAIR_DROP_READY(e) e->unk_500.part8.unk_501
#define ALTAIR_FLIGHT_PHASE(e) e->unk_4FC.part8.unk_4FC
#define ALTAIR_SPAWNER(e) e->unk_504.unk_504_entity
#define ALTAIR_SPAWN_COUNT(e) e->unk_500.part16.unk_502

/*
 * Drop counter: Altair_CreateCarriedEnemy counts each carried enemy on the
 * Altair spawner (drop count) and marks the enemy (drop counted, with the
 * spawner's entity index); the enemy decrements the count when it dies.
 */
#define ALTAIR_DROP_COUNT(e) e->unk_4FC.part8.unk_4FD
#define ALTAIR_DROP_COUNTED(e) e->unk_4FC.part8.unk_4FD
#define ALTAIR_DROP_SPAWNER_INDEX(e) e->unk_4F4.part16.unk_4F6

/*
 * Feathers (AltairFeather_Update): the Altair that shed it, sway speed (random
 * 0x40..0xBF) and the sway phase it advances (ends after half a turn).
 */
#define ALTAIR_FEATHER_OWNER(e) e->unk_4F4.unk_4F4_entity
#define ALTAIR_FEATHER_SWAY_SPEED(e) e->unk_4F8.part16.unk_4F8
#define ALTAIR_FEATHER_SWAY_PHASE(e) e->unk_4F8.part16.unk_4FA

/* Feather: random spin and a speed set when it starts to fall; never read. */
#define ALTAIR_FEATHER_UNUSED_SPIN(e) e->unk_508.part16.unk_508
#define ALTAIR_FEATHER_UNUSED_SPEED(e) e->unk_508.part16.unk_50A

#endif /* ALTAIR_ENTITY_H */
