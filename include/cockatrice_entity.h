#ifndef COCKATRICE_ENTITY_H
#define COCKATRICE_ENTITY_H

/*
 * Cockatrice / Basilisk (CockatriceFamily*):
 * - body: palette slot shared with its part and attack effect; death flags
 *   (bit 0 set when the death sequence starts, ends the part).
 */

/*
 * Swinging affine part (CockatriceFamilyPart_Update): affine angle, swing
 * phase, x scale and swing amplitude (8, 16 while the body attacks).
 */

/*
 * Petrifying beam (CockatriceFamilyAttack_*): affine angle, frame counter
 * (ends after 32 frames), y scale, length (scale of the step) and the step
 * between the 16 copies drawn by CockatriceFamilyAttack_Draw.
 */
#define COCKATRICE_BEAM_STEP_Y(e) e->unk_510.whole

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

#endif /* COCKATRICE_ENTITY_H */
