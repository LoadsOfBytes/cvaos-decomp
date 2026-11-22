#ifndef GHOST_ENTITY_H
#define GHOST_ENTITY_H

/*
 * Ghost spawner (EnemyGhostUpdate state 3): number of live ghosts (at most
 * spawnParam0; a ghost decrements its owner's count when it is deleted) and
 * the frames since the last spawn (spawnParam1 minimum, then 1 in 32).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GHOST_CHILD_COUNT(e) e->unk_4FC.part16.unk_4FC
#define GHOST_SPAWN_DELAY(e) e->unk_4FC.part16.unk_4FE[0]

#endif /* GHOST_ENTITY_H */
