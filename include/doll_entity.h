#ifndef DOLL_ENTITY_H
#define DOLL_ENTITY_H

/*
 * Killer Doll / Lightning Doll death (KillerDoll_*, LightningDoll_*): the doll
 * counts its live death parts (spawned by *_SpawnDeathParts, each part
 * decrements it when it ends); a part keeps its index (row of
 * s*DollPartOffsets) and the entity index of the effect it spawned.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define DOLL_DEATH_PART_COUNT(e) e->unk_500.part8.unk_500
#define DOLL_PART_INDEX(e) e->unk_500.part8.unk_501
#define DOLL_PART_EFFECT_INDEX(e) e->unk_4F4.part8.unk_4F6

/*
 * Active dolls: countdowns checked while walking (count down every frame).
 * Killer Doll: until it may lunge at the player, until its turn animation,
 * until it sinks to the floor and rests. Lightning Doll: until it may attack,
 * frames of the current move, until it rests.
 */
#define KILLER_DOLL_LUNGE_TIMER(e) e->unk_4F8.part16.unk_4F8
#define KILLER_DOLL_TURN_TIMER(e) e->unk_4F8.part16.unk_4FA
#define KILLER_DOLL_REST_TIMER(e) e->unk_4FC.part16.unk_4FC
#define LIGHTNING_DOLL_ATTACK_TIMER(e) e->unk_4F8.part16.unk_4F8
#define LIGHTNING_DOLL_MOVE_TIMER(e) e->unk_4F8.part16.unk_4FA
#define LIGHTNING_DOLL_REST_TIMER(e) e->unk_4FC.part16.unk_4FC

#endif /* DOLL_ENTITY_H */
