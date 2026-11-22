#ifndef EVENT_SCENE_ENTITY_H
#define EVENT_SCENE_ENTITY_H

/*
 * Small event scenes:
 * - scene 23 (EventScene23_Init): the actor created from object table kind 1;
 * - scene 26 (EventScene26_*): the actor created from object table kind 3
 *   (drawn with EventScene26_DrawOffsetCopies) and the text id it shows;
 * - scene 27 (town character, EventScene27_*): text id of the picked
 *   conversation and the heal flag (restores HP / MP on the next talk);
 * - scene 31 (EventScene31_StepPaletteAnim): frames shown of the current
 *   palette step and the step 0..3.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define EVENT23_ACTOR(e) e->unk_4F8.unk_4F8_entity
#define EVENT26_ACTOR(e) e->unk_4FC.unk_4FC_entity
#define EVENT26_TEXT_ID(e) e->unk_4F8.whole
#define EVENT27_TEXT_ID(e) e->unk_4F8.part16.unk_4F8
#define EVENT27_HEAL(e) e->unk_4FC.bytes[0]
#define EVENT31_PALETTE_TICK(e) e->unk_4FC.part16.unk_4FC
#define EVENT31_PALETTE_STEP(e) e->unk_4FC.part16.unk_4FE[0]

/* Scene 28 (EventScene28_*): text id of the picked script. */
#define EVENT28_TEXT_ID(e) e->unk_4F8.whole

/*
 * Scene 31 (EventScene31_Init / _Update): the block holder entity it created.
 * Scene 24 (EventScene24_Update): the answer of the yes / no question.
 */
#define EVENT31_BLOCK_HOLDER(e) e->unk_4F8.unk_4F8_entity
#define EVENT24_ANSWER(e) e->unk_4F4.part8.unk_4F7

#endif /* EVENT_SCENE_ENTITY_H */
