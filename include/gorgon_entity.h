#ifndef GORGON_ENTITY_H
#define GORGON_ENTITY_H

/*
 * Gorgon work slots:
 * - flags: bit 0 = show the forced frame instead of the animation (cleared
 *   every update);
 * - forced frame: the sprite frame shown while bit 0 is set;
 * - hit: set by Gorgon_OnHit, cleared by the update;
 * - breath angle / breath dir / breath step: the breath attack sweeps its
 *   angle by 0x400 per frame and reverses the direction every 7 steps.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define GORGON_FLAGS(e) e->unk_4FC.part8.unk_4FC
#define GORGON_FORCED_FRAME(e) e->unk_4FC.part8.unk_4FE
#define GORGON_HIT(e) e->unk_500.part8.unk_500
#define GORGON_BREATH_ANGLE(e) e->unk_504.part16.unk_506[0]
#define GORGON_BREATH_DIR(e) e->unk_504.part8.unk_504
#define GORGON_BREATH_STEP(e) e->unk_504.part8.unk_505

/*
 * Walk (Gorgon_UpdateWalk; TU aliases GAIT / FRAME / CLOCK for bytes
 * unk_4FD / unk_4FE / unk_4FF): the four bytes flags / gait / frame / clock
 * read as one word (masked with 0xFF00FF00 to test gait and clock) and the
 * frame / clock halfword (frame | clock << 8).
 */
#define GORGON_WALK_WORD(e) e->unk_4FC.whole
#define GORGON_FRAME_CLOCK(e) e->unk_4FC.part16.unk_4FE[0]

/* Walk gait and frame clock bytes (TU aliases GAIT / CLOCK; cleared after the
 * breath attack). */
#define GORGON_WALK_GAIT(e) e->unk_4FC.part8.unk_4FD
#define GORGON_WALK_CLOCK(e) e->unk_4FC.part8.unk_4FF

#endif /* GORGON_ENTITY_H */
