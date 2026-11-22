#ifndef MERMAN_ENTITY_H
#define MERMAN_ENTITY_H

/*
 * Merman (EnemyMerman*, Merman_*):
 * - leap / land splashed: the water-exit and water-entry splash effects of
 *   the current jump were spawned;
 * - swim mode: 0 = wander, 1 = face the target;
 * - walk count: walk turns on land (back into the water after 8);
 * - home x / y: room pixel position of the spawn point (the water line);
 * - spawner (spawnParam1 != 0) keeps its live children in child count, each
 *   child links back to it through spawner.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define MERMAN_LEAP_SPLASHED(e) e->unk_4F8.part8.unk_4F8
#define MERMAN_SWIM_MODE(e) e->unk_4F8.part8.unk_4F9
#define MERMAN_LAND_SPLASHED(e) e->unk_4F8.part8.unk_4FA
#define MERMAN_WALK_COUNT(e) e->unk_4F8.part8.unk_4FB
#define MERMAN_HOME_X(e) e->unk_500.part16.unk_500
#define MERMAN_HOME_Y(e) e->unk_500.part16.unk_502
#define MERMAN_CHILD_COUNT(e) e->unk_4FC.whole
#define MERMAN_SPAWNER(e) e->unk_4FC.unk_4FC_entity

#endif /* MERMAN_ENTITY_H */
