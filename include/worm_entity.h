#ifndef WORM_ENTITY_H
#define WORM_ENTITY_H

/*
 * Worm (Worm_Init builds a root entity and a chain of segments):
 * - root: every segment (and the root itself) points to the root;
 * - previous: each segment follows the previous one;
 * - segment index: position in the chain (0 = head end, 4 = tail);
 * - crawl dir: direction the root crawls (0 / 1), turn timer: frames until it
 *   picks a direction again;
 * - variant: the root's variant (0 = ground worm, else it undulates);
 * - aggressive: set on the root when it starts chasing;
 * - wave phase: per-segment phase of the undulation.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define WORM_ROOT(e) e->unk_4F8.unk_4F8_entity
#define WORM_PREVIOUS(e) e->unk_4FC.unk_4FC_entity
#define WORM_SEGMENT_INDEX(e) e->unk_500.part8.unk_502
#define WORM_CRAWL_DIR(e) e->unk_500.part8.unk_503
#define WORM_TURN_TIMER(e) e->unk_500.part8.unk_501
#define WORM_VARIANT(e) e->unk_504.part8.unk_505
#define WORM_AGGRESSIVE(e) e->unk_500.part8.unk_500
#define WORM_WAVE_PHASE(e) e->unk_504.part16.unk_506[0]
/* Entity slot of the next body segment (Worm_Init, followed by Worm_Draw). */
#define WORM_NEXT_INDEX(e) e->unk_504.part8.unk_504

#endif /* WORM_ENTITY_H */
