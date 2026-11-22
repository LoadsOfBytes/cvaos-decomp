#ifndef MANTICORE_ENTITY_H
#define MANTICORE_ENTITY_H

/*
 * Manticore tail: the body keeps the tail tip; each tail segment (and the tip)
 * keeps the next segment towards the tip, its anchor part, its kind (0..5
 * segments, 6 = tip) and a value used by the swing (tip: initial 0x4000).
 * Tail entities keep the Manticore itself in ENTITY_OWNER.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define MANTICORE_TAIL_TIP(e) e->unk_4F8.unk_4F8_entity
#define MANTICORE_TAIL_NEXT(e) e->unk_504.unk_504_entity
#define MANTICORE_TAIL_ANCHOR(e) e->unk_4FC.unk_4FC_entity
#define MANTICORE_TAIL_KIND(e) e->unk_500.part8.unk_500
#define MANTICORE_TAIL_VALUE(e) e->unk_504.whole

/* Shot attack (Manticore_UpdateAttack): shots fired and shots to fire (1..3). */
#define MANTICORE_SHOT_COUNT(e) e->unk_508.part8.unk_508
#define MANTICORE_SHOT_TOTAL(e) e->unk_508.part8.unk_509

/*
 * Tail swing (ManticoreTail_UpdateSwing - TU aliases TIMER / STEP): frame
 * counter and step of the current swing sequence.
 */
#define MANTICORE_TAIL_TIMER(e) e->unk_500.part8.unk_502
#define MANTICORE_TAIL_STEP(e) e->unk_500.part8.unk_503

/*
 * Tail chain update (code_080733D4.c - TU aliases KIND / DONE / NEXT): set
 * when a segment has been updated this frame; the base segment (kind 0)
 * clears it along the chain and each segment then updates the next one.
 */
#define MANTICORE_TAIL_UPDATED(e) e->unk_500.part8.unk_501

#endif /* MANTICORE_ENTITY_H */
