#ifndef MINOTAUR_ENTITY_H
#define MINOTAUR_ENTITY_H

/*
 * Minotaur (Minotaur_*): animation step copied each frame (read by the swing
 * trail), variant (0 = attacks without the distance check), flags (bit 0 =
 * the axe follows the hand, bit 1 = it still has its axe; both cleared when
 * hit to death).
 */
#define MINOTAUR_ANIM_STEP(e) e->unk_4F8.part8.unk_4F8
#define MINOTAUR_VARIANT(e) e->unk_4F8.part8.unk_4F9
#define MINOTAUR_FLAGS(e) e->unk_4FC.whole

/*
 * Axe (MinotaurAxe_Update; spin, spin speed and dropped flag are TU macros
 * there): its angle, read by MinotaurAxeHitBox_Update. Swing trail
 * (MinotaurSwingTrail_Update): affine angle.
 */
#define MINOTAUR_AXE_ANGLE(e) e->unk_4FC.part16.unk_4FE[0]
#define MINOTAUR_TRAIL_ANGLE(e) e->unk_4FC.part16.unk_4FC

/*
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */

/*
 * Axe (MinotaurAxe_Update - TU aliases SPIN / SPIN_SPEED / DROPPED): affine
 * angle and spin speed of the falling axe and the dropped flag; it follows the
 * owner's MINOTAUR_ANIM_STEP while held.
 */
#define MINOTAUR_AXE_SPIN(e) e->unk_4FC.part16.unk_4FC
#define MINOTAUR_AXE_SPIN_SPEED(e) e->unk_500.part16.unk_500
#define MINOTAUR_AXE_DROPPED(e) e->unk_500.part16.unk_502

#endif /* MINOTAUR_ENTITY_H */
