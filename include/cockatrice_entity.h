#ifndef COCKATRICE_ENTITY_H
#define COCKATRICE_ENTITY_H

/*
 * Cockatrice / Basilisk (CockatriceFamily*):
 * - body: palette slot shared with its part and attack effect; death flags
 *   (bit 0 set when the death sequence starts, ends the part).
 */
#define COCKATRICE_PALETTE_SLOT(e) e->unk_4F8.part8.unk_4F8
#define COCKATRICE_DEATH_FLAGS(e) e->unk_4FC.whole

/*
 * Swinging affine part (CockatriceFamilyPart_Update): affine angle, swing
 * phase, x scale and swing amplitude (8, 16 while the body attacks).
 */
#define COCKATRICE_PART_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define COCKATRICE_PART_PHASE(e) e->unk_4FC.part16.unk_4FE[0]
#define COCKATRICE_PART_SCALE_X(e) e->unk_500.whole
#define COCKATRICE_PART_AMPLITUDE(e) e->unk_504.part16.unk_504

/*
 * Petrifying beam (CockatriceFamilyAttack_*): affine angle, frame counter
 * (ends after 32 frames), y scale, length (scale of the step) and the step
 * between the 16 copies drawn by CockatriceFamilyAttack_Draw.
 */
#define COCKATRICE_BEAM_ANGLE(e) e->unk_4FC.part16.unk_4FC
#define COCKATRICE_BEAM_FRAME(e) e->unk_4FC.part16.unk_4FE[0]
#define COCKATRICE_BEAM_SCALE_Y(e) e->unk_500.whole
#define COCKATRICE_BEAM_LENGTH(e) e->unk_508.whole
#define COCKATRICE_BEAM_STEP_X(e) e->unk_50C.whole
#define COCKATRICE_BEAM_STEP_Y(e) e->unk_510.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* COCKATRICE_ENTITY_H */
