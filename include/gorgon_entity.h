#ifndef GORGON_ENTITY_H
#define GORGON_ENTITY_H

/*
 * Gorgon work slots are in struct GorgonWork (ext.gorgon). The walk code
 * (Gorgon_UpdateWalk) also reads overlapping views that do not fit a member:
 * the four bytes flags / gait / forcedFrame / clock as one word (masked with
 * 0xFF00FF00 to test gait and clock) and the forcedFrame / clock halfword
 * (frame | clock << 8).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GORGON_WALK_WORD(e) e->ext.raw.unk_4FC.whole
#define GORGON_FRAME_CLOCK(e) e->ext.raw.unk_4FC.part16.unk_4FE[0]

#endif /* GORGON_ENTITY_H */
