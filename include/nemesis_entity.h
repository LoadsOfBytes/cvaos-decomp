#ifndef NEMESIS_ENTITY_H
#define NEMESIS_ENTITY_H

/*
 * Work slots of Nemesis (and of its partner entity, which runs the same code):
 * - angle: heading / affine rotation, turn: rotation added per frame;
 * - flags: state bits shared by the attack routines (2, 0x20, ...);
 * - partner index: entity index of the partner (Nemesis_CreatePartner);
 * - afterimage 0..2: entity slots of its three afterimages;
 * - hits before react: hits it absorbs before the hit reaction (reset to 3);
 * - wait timer: random wait while positioning;
 * - attack set / index: attack table selection (Nemesis_UpdateAttackSelection).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define NEMESIS_ANGLE(e) e->unk_500.part16.unk_500
#define NEMESIS_TURN(e) e->unk_500.part16.unk_502
#define NEMESIS_FLAGS(e) e->unk_4FC.part16.unk_4FC
#define NEMESIS_PARTNER_INDEX(e) e->unk_4F4.part8.unk_4F6
#define NEMESIS_AFTERIMAGE_0(e) e->unk_504.part8.unk_505
#define NEMESIS_AFTERIMAGE_1(e) e->unk_504.part8.unk_506
#define NEMESIS_AFTERIMAGE_2(e) e->unk_504.part8.unk_507
#define NEMESIS_HITS_BEFORE_REACT(e) e->unk_508.part8.unk_508
#define NEMESIS_WAIT_TIMER(e) e->unk_4F8.part16.unk_4F8
#define NEMESIS_ATTACK_SET(e) e->unk_4F8.part8.unk_4FA
#define NEMESIS_ATTACK_INDEX(e) e->unk_4F8.part8.unk_4FB

/*
 * Phase / timer of the current movement (TU aliases PHASE / TIMER): rocking
 * phase while approaching, arc timer, recoil phase.
 */
#define NEMESIS_PHASE(e) e->unk_4FC.part16.unk_4FE[0]

#endif /* NEMESIS_ENTITY_H */
