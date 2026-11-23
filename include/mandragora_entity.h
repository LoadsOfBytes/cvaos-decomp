#ifndef MANDRAGORA_ENTITY_H
#define MANDRAGORA_ENTITY_H

/*
 * Mandragora (Mandragora_*, MandragoraPart_*): the body and each stem part
 * keep the entity index of the next linked part (towards the tip); the body
 * counts its live death parts (MandragoraDeathPart_Update decrements it).
 */
#define MANDRAGORA_LINK_INDEX(e) e->unk_4F4.part8.unk_4F6
#define MANDRAGORA_DEATH_PARTS(e) e->unk_4F4.part8.unk_4F7

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/* Flag byte set by its parts (TU aliases FLAGS): 2, 8, 0x10, 0x20 - bits 3 / 4
 * end the scream (Mandragora_UpdateScream). */
#define MANDRAGORA_FLAGS(e) e->unk_504.part8.unk_504

#endif /* MANDRAGORA_ENTITY_H */
