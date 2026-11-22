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

#endif /* DOLL_ENTITY_H */
