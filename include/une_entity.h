#ifndef UNE_ENTITY_H
#define UNE_ENTITY_H

/*
 * Une: the room entity (EnemyUneCreate) is the spawner; every plant points to
 * it (spawner) and the spawner counts the live plants (count, limited to its
 * spawnParam0). Plants: original = 1 for the first plant, spread timer = frames
 * until the next plant grows.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define UNE_SPAWNER(e) e->unk_4FC.unk_4FC_entity
#define UNE_COUNT(e) e->unk_4FC.whole
#define UNE_ORIGINAL(e) e->unk_500.whole
#define UNE_SPREAD_TIMER(e) e->unk_504.whole

#endif /* UNE_ENTITY_H */
