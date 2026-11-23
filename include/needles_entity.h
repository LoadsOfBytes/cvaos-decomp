#ifndef NEEDLES_ENTITY_H
#define NEEDLES_ENTITY_H

/*
 * Needles: bob phase of the floating variant (spawn parameter 0 nonzero;
 * random start, +0x400 per frame).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define NEEDLES_BOB_PHASE(e) e->unk_4F8.part16.unk_4F8

/* Death (Needles_SpawnDeathParts / NeedlesPart_Update / Needles_UpdateDeath):
 * live death parts; each part decrements it when it ends. */
#define NEEDLES_DEATH_PARTS(e) e->unk_4F4.part8.unk_4F7

#endif /* NEEDLES_ENTITY_H */
