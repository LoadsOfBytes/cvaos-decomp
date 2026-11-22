#ifndef BALORE_ENTITY_H
#define BALORE_ENTITY_H

/*
 * Balore body: flags (1 = hit, 2 = hurt reaction, 4 = dying, 8 / 0x10 = left /
 * right hand finished, 0x20), the ObjGfx slot of its sprite sheet (shared by
 * all parts) and the command for the hands (state they switch to).
 * Its parts keep a pointer to the body; the hands' hit boxes keep a pointer to
 * their hand. Hands: arm length (16.16, the hit box sits at its end), arm
 * angle and the horizontal scale of the stretched arm sprite.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BALORE_FLAGS(e) e->unk_500.part16.unk_502
#define BALORE_GFX(e) e->unk_4F8.part8.unk_4FB
#define BALORE_HAND_COMMAND(e) e->unk_4FC.part8.unk_4FC
#define BALORE_BODY(e) e->unk_4FC.unk_4FC_entity
#define BALORE_HAND(e) e->unk_4F8.unk_4F8_entity
#define BALORE_ARM_LENGTH(e) e->unk_4F4.whole
#define BALORE_ARM_ANGLE(e) e->unk_500.part16.unk_500
#define BALORE_ARM_SCALE(e) e->unk_504.whole

/*
 * Affine effect (BaloreAffineEffect_Update): angle, spin step and scale.
 * Intro piece (BaloreIntroPiece_Update): piece index (start angle, orbit
 * speed table index), orbit angle and the shrinking orbit radius.
 */
#define BALORE_EFFECT_ANGLE(e) e->unk_500.part16.unk_500
#define BALORE_EFFECT_SPIN(e) e->unk_500.part16.unk_502
#define BALORE_EFFECT_SCALE(e) e->unk_504.whole
#define BALORE_PIECE_INDEX(e) e->unk_504.part8.unk_504
#define BALORE_PIECE_ANGLE(e) e->unk_500.part16.unk_500
#define BALORE_PIECE_RADIUS(e) e->unk_500.part16.unk_502

/* Hands: room position they return to (set by Balore_CreateHand). */
#define BALORE_HAND_HOME_X(e) e->unk_508.part16.unk_508
#define BALORE_HAND_HOME_Y(e) e->unk_508.part16.unk_50A

/*
 * Body: affine scale (1.0, shrinks while dying; the eye scales its aim offset
 * with it), rotation angle (read by the overlay), eye / form animation index
 * (4 at first; parts pick their animation offsets with it) and palette slot.
 */
#define BALORE_BODY_SCALE(e) e->unk_504.whole
#define BALORE_BODY_ANGLE(e) e->unk_500.part16.unk_500
#define BALORE_EYE_ANIM(e) e->unk_4F4.part8.unk_4F6
#define BALORE_PALETTE(e) e->unk_4F8.part8.unk_4F8

/*
 * Intro (Balore_CreateIntro): its 32 pieces (BaloreIntroPiece_Update) keep the
 * intro entity; their index is BALORE_PIECE_INDEX.
 */
#define BALORE_PIECE_OWNER(e) e->unk_4FC.unk_4FC_entity

#endif /* BALORE_ENTITY_H */
